#include "perf_measurements.h"

#include "performance_helpers.h"
#include "proc_handler.h"
#include "src/benchmarks/analysis/correlation.h"
#include "src/benchmarks/linux/xavier_sensors.h"
#include "src/exports/export.h"
#include "src/exports/export_types/export_csv.h"
#include "src/exports/export_types/export_graphs.h"
#include "src/exports/export_types/export_json.h"
#include "src/exports/export_types/summary_generator.h"
#include "src/globals.h"
#include "src/helpers/logger.h"
#include "src/helpers/stopwatch.h"
#include "src/helpers/synchronizer.h"
#include "src/json_config/sensor_config/config_parser.h"
#include "src/linux/filesystem.h"

#include <algorithm>
#include <iostream>
#include <regex>       // std::regex_replace
#include <sys/types.h> // getpid()
#include <thread>
#include <unistd.h> // getpid()

namespace Linux
{
CPerfMeasurements::CPerfMeasurements(Synchronizer* synchronizer,
                                     const std::string& sensorConfig,
                                     const std::vector<Core::SThreshold>& thresholds)
: processes_{ nullptr }
, threadSync_{ synchronizer }
, sensorConfigFile_{ sensorConfig }
, sensorMeasurements_{ sensorConfig }
, processMeasurements_{ sensorConfig }
, thresholds_{ thresholds }
{
}

/**
 * @brief Will initiate and start the measurements loop
 */
void CPerfMeasurements::Start(const Core::SConfig& config, std::vector<ProcessInfo>* processes)
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
  if (config_.settings.enableLiveMode)
    exportObj_.FinishLiveMeasurements();
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
  gstMeasurements_.setProctime(config_.settings.enableProcTime);
  gstMeasurements_.SetConfig(config_);

  if (config_.settings.enableLiveMode)
    exportObj_.InitialiseLiveMeasurements(&allSensors_, config_);

  sensorMeasurements_.Initialize(&measurementsData_);
  gstMeasurements_.Initialize(&measurementsData_);
  std::vector<RunProcess*> linuxProcesses = GetProcessFromProcesses<RunProcess>();
  processMeasurements_.Initialize(&measurementsData_, linuxProcesses);

  allSensors_.AddSensors(Measurements::EClassification::SYSTEM, sensorMeasurements_.GetSensors(false));
  allSensors_.AddSensors(Measurements::EClassification::PROCESSES, processMeasurements_.GetSensors(false));

  std::this_thread::sleep_for(std::chrono::milliseconds(config_.settings.measureLoopMs));
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
  testRunningTimer_.Restart();
  while (!threadSync_->AllCompleted())
  {
    // Filling the export data
    Measurements::SMeasurementsData measurementData;

    measurementData.time = std::to_string(testRunningTimer_.GetTime<std::milli>()); // Millisecond accuracy

    // Get the measurements from each source
    measurementData.AddMeasurements(Measurements::EClassification::SYSTEM, sensorMeasurements_.GetMeasurements());
    measurementData.AddMeasurements(Measurements::EClassification::PROCESSES, processMeasurements_.GetMeasurements());
    measurementData.AddMeasurements(Measurements::EClassification::PIPELINE, gstMeasurements_.ProcessGstreamer());

    // Measure data on each GStreamer pipeline
    measurementsData_.push_back(measurementData);

    // Send it to the live exports
    if (config_.settings.enableLiveMode)
    {
      // Have to add the sensors of the pipeline each time as they are
      // dynamically added (the old sensors are cleared when adding new ones)
      allSensors_.AddSensors(Measurements::EClassification::PIPELINE, gstMeasurements_.GetSensors(false));
      exportObj_.AddMeasurements(measurementData);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(config_.settings.measureLoopMs));
  }
  // Sort the pipeline measurements correctly
  for (auto& e : measurementsData_)
  {
    auto newGroups = gstMeasurements_.SortData(e.GetItemGroups(Measurements::EClassification::PIPELINE));
    e.AddMeasurements(Measurements::EClassification::PIPELINE, newGroups);
  }
}

void CPerfMeasurements::ExportData(const Exports::AllSensors& sensors,
                                   const std::vector<Measurements::CCorrelation::SResult>& correlationResults)
{
  std::vector<Exports::SMeasurementItem> items;
  items.push_back(sensorMeasurements_.GetConfig());
  items.push_back(gstMeasurements_.GetPipelineConfig2());
  items.push_back(processMeasurements_.GetConfig());

  Exports::CExport exportGenericClass;
  Exports::SExportData expData{ items, &measurementsData_, sensors, correlationResults };
  for (const auto& e : config_.settings.exports)
    exportGenericClass.ExecuteExport(e, expData, config_);
}

/**
 * @brief Collects all the data from the performed measurements, executes some analysis and sends this data to the
 * export methods
 */
void CPerfMeasurements::AnalyzeData()
{
  // Collect all sensors
  auto gstSensors = gstMeasurements_.GetSensors();
  auto sysSensors = sensorMeasurements_.GetSensors();
  auto processSensors = processMeasurements_.GetSensors();

  Exports::AllSensors allSensors;
  allSensors.AddSensors(Measurements::EClassification::PIPELINE, gstSensors);
  allSensors.AddSensors(Measurements::EClassification::SYSTEM, sysSensors);
  allSensors.AddSensors(Measurements::EClassification::PROCESSES, processSensors);

  // Execute the correlation check here
  auto corrResults =
    Measurements::CCorrelation::GetCorrelation(allSensors, &measurementsData_, config_.settings.enablePretestZeroes);

  // Check thresholds
  SetThresholdResults(allSensors);

  ExportData(allSensors, corrResults);
}

void CPerfMeasurements::SetThresholdResults(const Measurements::SAllSensors& allSensors)
{
  auto allProcessIds = allSensors.GetProcesses();
  for (const auto& processId : allProcessIds)
  {
    auto sensorMap = allSensors.GetMap(processId);

    for (const auto& e : thresholds_)
    {
      if (e.processId != processId)
        continue; // This process doesn't need checking for this threshold
      auto sensor = sensorMap.find(e.name);
      if (sensor == sensorMap.end())
      {
        CLogger::Log(CLogger::Types::WARNING, "Threshold name: \"", e.name, "\" not found!");
        continue; // Couldn't check any thresholds, doesn't exist
      }
      if (!sensor->second->thresholdExceeded)
        sensor->second->thresholdExceeded = PerformanceHelpers::HandleThreshold(sensor->second, e);
      // }
    }
  }
}

} // namespace Linux