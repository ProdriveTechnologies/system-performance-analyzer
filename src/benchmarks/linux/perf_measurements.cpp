#include "perf_measurements.h"

#include <algorithm>
#include <iostream>
#include <regex>       // std::regex_replace
#include <sys/types.h> // getpid()
#include <thread>
#include <unistd.h> // getpid()

#include "Helpers.h"
#include "src/benchmarks/linux/xavier_sensors.h"
#include "src/helpers/synchronizer.h"
#include "src/linux/filesystem.h"

#include "src/exports/export.h"
#include "src/exports/export_types/export_csv.h"
#include "src/exports/export_types/export_json.h"

#include "proc_handler.h"
#include "src/globals.h"
#include "src/helpers/logger.h"
#include "src/helpers/stopwatch.h"
#include "src/json_config/sensor_config/config_parser.h"
// #include "xavier_sensors_live.h"

#include "performance_helpers.h"
#include "src/exports/export_types/summary_generator.h"

#include "src/benchmarks/analysis/correlation.h"

namespace Linux
{
// TODO: Make cpuUtilizationTimer_ configurable through the JSON
CPerfMeasurements::CPerfMeasurements(
    Synchronizer *synchronizer, const std::string &sensorConfig,
    const std::vector<Core::SThreshold> &thresholds)
    : threadSync_{synchronizer}, sensorConfigFile_{sensorConfig},
      cpuUtilizationTimer_{std::chrono::milliseconds{1000}},
      sensorMeasurements_{sensorConfig}, processMeasurements_{sensorConfig},
      thresholds_{thresholds} //,
// liveFilesystemData_{std::chrono::milliseconds{1000}, XAVIER_CORES}
{
}

/**
 * @brief Will initiate and start the measurements loop
 *
 * @param config
 */
void CPerfMeasurements::Start(const Core::SConfig &config,
                              std::vector<ProcessInfo> *processes)
{
  processes_ = processes;
  config_ = config;
  Stopwatch stopwatch; // Stopwatch keeps track of how long the test is running
  CLogger::Log(CLogger::Types::INFO, "Starting synchronize 1 for benchmarks");
  threadSync_->WaitForProcess();

  // Initialization for the performance measurements
  Initialize();

  // Sync before start, when synced, will start directly
  CLogger::Log(CLogger::Types::INFO, "Starting synchronize 2 for benchmarks");
  threadSync_->WaitForProcess();

  stopwatch.Start();
  // Start the actual measurements loop
  StartMeasurementsLoop();

  // And synchronize the stop, so all threads stop at the same time
  threadSync_->WaitForProcess();

  // Finish the analysis
  AnalyzeData();
}

/**
 * @brief Initialization of the performance measurements
 * Executed between the first and second synchronization with the external
 * applications
 */
void CPerfMeasurements::Initialize()
{
  OrganizeGstreamerPipelines();

  pMeasurementsData_ = std::make_unique<std::vector<Exports::ExportData>>();
  pCpuData_ = std::make_unique<std::vector<Linux::FileSystem::ProcStatData>>();
  pProcessesData_ = std::make_unique<std::vector<ProcessesMeasure>>();

  // Must happen after the creation of the pMeasurementsData_ memory block
  sensorMeasurements_.Initialize(pMeasurementsData_.get());
  gstMeasurements_.Initialize(pMeasurementsData_.get());
  std::vector<RunProcess *> linuxProcesses =
      GetProcessFromProcesses<RunProcess>();
  processMeasurements_.Initialize(pMeasurementsData_.get(), linuxProcesses);

  // Thread exists now, store threads that already exist for the monitoring. The
  // newly added ones are from gstreamer and need to be monitored
  // std::cerr << "Monitoring thread: " << threadSync_->getThreadId();

  // Path processPath;
  // processPath.AddItems("proc", getpid(), "task");
  // excludedThreads_ = FileSystem::GetFiles(processPath.GetPath());

  // Initialise proc stat
  // procHandler_.ParseProcStat();
  cpuUtilizationTimer_.restart();
  // lastCpuDataAggregated_ = Linux::FileSystem::GetProcStat(XAVIER_CORES);
  while (!cpuUtilizationTimer_.elapsed())
  {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(cpuUtilizationTimer_.timeTillElapsed()));
  }
  // liveFilesystemData_.Init();
}

/**
 * @brief Organizes the pipelines and configures the gstMeasurements_
 * accordingly
 */
void CPerfMeasurements::OrganizeGstreamerPipelines()
{
  auto gstPipelines = GetProcessFromProcesses<CGstreamerHandler>();
  gstMeasurements_.AddPipelines(gstPipelines);
}

/**
 * @brief Will start the loop that measures and stores the measurements
 *
 */
void CPerfMeasurements::StartMeasurementsLoop()
{
  // std::vector<std::string> monitoredThreads;
  testRunningTimer_.Restart();
  while (!threadSync_->AllCompleted())
  {
    // 1. get all threads that will be monitored
    // monitoredThreads = FileSystem::GetFiles(processPath.GetPath());
    // Helpers::RemoveIntersection(monitoredThreads, excludedThreads_);
    // // 2. Loop through threads and execute benchmarks on them

    // Filling the export data
    // procHandler_.ParseMeminfo(); // Parse the /proc/meminfo struct
    Exports::ExportData exportData;
    exportData.time = std::to_string(
        testRunningTimer_.GetTime<std::milli>()); // Millisecond accuracy
    exportData.measuredItems = sensorMeasurements_.GetMeasurements();
    exportData.processInfo = processMeasurements_.GetMeasurements();

    // Measure data on each GStreamer pipeline
    exportData.pipelineInfo = gstMeasurements_.ProcessGstreamer();

    pMeasurementsData_->push_back(exportData);

    std::this_thread::sleep_for(
        std::chrono::milliseconds(config_.settings.measureLoopMs));
  }
  // Sort the pipeline measurements correctly
  for (auto &e : *pMeasurementsData_)
  {
    e.pipelineInfo = gstMeasurements_.SortData(e.pipelineInfo);
  }
  exportConfig_.pipelineConfig = gstMeasurements_.GetPipelineConfig();
}

void CPerfMeasurements::ExportData(const Exports::AllSensors &sensors)
{
  std::vector<Exports::MeasurementItem> items;
  items.push_back(sensorMeasurements_.GetConfig());
  items.push_back(gstMeasurements_.GetPipelineConfig2());
  items.push_back(processMeasurements_.GetConfig());

  ExecuteExport<Exports::CSummaryGenerator>("filename", items, sensors);
  ExecuteExport<Exports::CCsv>("filename", items, sensors);
}

/**
 * @brief Remove process id from the processPids_ vector
 */
void CPerfMeasurements::RemoveProcessId(const int pid)
{
  for (size_t i = 0; i < processPids_.size(); i++)
  {
    const auto &e = processPids_[i];
    if (e == pid)
      processPids_.erase(processPids_.begin() + i);
  }
}

void CPerfMeasurements::AnalyzeData()
{
  /**
   * @brief Steps to execute:
   * 1. Summarize the necessary data (also min, max, average)
   * 2. Check thresholds on said summarized data
   * 3. Add boolean to say whether the test failed/was succesful
   * 4. Execute correlation checks nevertheless. If failed, check correlation
   * between performance and resources
   * 5. Create report with the summarized data. Exports should have access to
   * the analyzed data as well. Maybe create one structure that can be extended
   * for this?
   */

  // Collect all sensors
  auto gstSensors = gstMeasurements_.GetSensors();
  auto sysSensors = sensorMeasurements_.GetSensors();
  auto processSensors = processMeasurements_.GetSensors();

  Exports::AllSensors allSensors;
  allSensors.AddSensors(Measurements::Classification::PIPELINE, gstSensors);
  allSensors.AddSensors(Measurements::Classification::SYSTEM, sysSensors);
  allSensors.AddSensors(Measurements::Classification::PROCESSES,
                        processSensors);

  // Execute the correlation check here
  auto corrResults = Measurements::CCorrelation::GetCorrelation(
      allSensors, pMeasurementsData_.get());

  for (const auto &e : corrResults)
  {
    if (e.correlation >= 0.75 || e.correlation <= -0.75)
    {
      std::cout << "THESE SENSORS ARE CORRELATED!::" << std::endl;
      std::cout << "Correlation between " << e.sensor1.userId << " and "
                << e.sensor2.userId << " is: " << e.correlation << std::endl;
    }
  }

  // Check thresholds
  SetThresholdResults(allSensors);

  ExportData(allSensors);
}

void CPerfMeasurements::SetThresholdResults(Measurements::AllSensors allSensors)
// std::unordered_map<std::string, Measurements::Sensors> &sensorMap)
{
  auto allProcessIds = allSensors.GetProcesses();
  for (const auto &processId : allProcessIds)
  {
    auto sensorMap = allSensors.GetMap(processId);

    for (const auto &e : thresholds_)
    {
      if (e.processId != processId)
        continue; // This process doesn't need checking for this threshold
      auto sensor = sensorMap.find(e.name);
      if (sensor == sensorMap.end())
      {
        CLogger::Log(CLogger::Types::WARNING, "Threshold name: \"", e.name,
                     "\" not found!");
        continue; // Couldn't check any thresholds, doesn't exist
      }
      if (!sensor->second->thresholdExceeded)
        sensor->second->thresholdExceeded =
            PerformanceHelpers::HandleThreshold(sensor->second, e);
      // }
    }
  }
}

} // namespace Linux