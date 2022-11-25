#include "perf_measurements.h"

#include <algorithm>
#include <iostream>
#include <regex>       // std::regex_replace
#include <sys/types.h> // getpid()
#include <thread>
#include <unistd.h> // getpid()

#include "Helpers.h"
#include "src/benchmarks/Linux/xavier_sensors.h"
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
  ExportData();
  AnalyzeData();
}

/**
 * @brief Initialization of the performance measurements
 * Executed between the first and second synchronization with the external
 * applications
 */
void CPerfMeasurements::Initialize()
{
  // auto parsed = PlatformConfig::Parse(sensorConfigFile_);
  // auto measureFields =
  //     GetFields(parsed.sensors, &CPerfMeasurements::GetMeasureFields, this);
  // measureFields_ = measureFields.fields;
  // measureFieldsDefinition_ = measureFields.definition;
  // auto processFields =
  //     GetFields(parsed.sensors, &CPerfMeasurements::GetProcessFields, this);
  // processFields_ = processFields.fields;
  // processFieldsDef_ = processFields.definition;
  InitExports(sensorMeasurements_.GetDefinition());

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
    // for (const auto &e : monitoredThreads)
    // {
    //   MeasureThread(e);
    // }
    // Filling the export data
    // procHandler_.ParseMeminfo(); // Parse the /proc/meminfo struct
    Exports::ExportData exportData;
    exportData.time = std::to_string(
        testRunningTimer_.GetTime<std::milli>()); // Millisecond accuracy
    exportData.measuredItems = sensorMeasurements_.GetMeasurements();
    exportData.processInfo = processMeasurements_.GetMeasurements();
    // GetMeasuredItems(measureFields_);

    // exportData.processInfo
    // Measure data on each process
    // MeasureProcesses(processPids_);

    // Measure data on each GStreamer pipeline
    exportData.pipelineInfo = gstMeasurements_.ProcessGstreamer();

    pMeasurementsData_->push_back(exportData);
    // pCpuData_->push_back(procHandler_.GetStats());

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

/**
 * @brief initializes the resources used for the measurements
 */
void CPerfMeasurements::InitExports(const MeasureFieldsDefType &config)
{
  if (config_.settings.enableLogs)
  {
    pExportObj_ = std::make_unique<Exports::CExport>(new Exports::CCsv{},
                                                     "jsonfile.json", true);
    // pExportObj_->InitExport(config);
  }
}

/**
 * @brief sends the exports data to the export objects
 *
 * @param data the measurements data
 */
// void CPerfMeasurements::SendExportsData(const Exports::ExportData &data)
// {
//   pExportObj_->DataExport(data);
// }

void CPerfMeasurements::MeasureThread(const std::string &threadProcLoc)
{
  auto stats = Linux::FileSystem::GetStats(threadProcLoc + "/stat");
  std::cout << "CPU usage: " << stats.cutime << std::endl;
  std::cout << "Minor Faults: " << stats.minFaults << std::endl;
}

void CPerfMeasurements::ExportData()
{
  // for (const auto &e : *pMeasurementsData_)
  // {
  //   SendExportsData(e);
  // }

  // Do the full export with all data
  std::vector<Exports::MeasurementItem> items;
  items.push_back(sensorMeasurements_.GetConfig());
  items.push_back(gstMeasurements_.GetPipelineConfig2());
  items.push_back(processMeasurements_.GetConfig());
  pExportObj_->FullExport(items, pMeasurementsData_.get(),
                          Measurements::AllSensors{});
}

/**
 * @brief Measures the resource usage and interesting data from the processes in
 * the processId list
 *
 * @param processIds
 */
// void CPerfMeasurements::MeasureProcesses(const std::vector<int> processIds)
// {
//   ProcessesMeasure measurements;
//   for (const auto &e : processIds)
//   {
//     try
//     {
//       ProcessMeasurements processMeasurement;
//       processMeasurement.pid = e;
//       processMeasurement.stats =
//           Linux::FileSystem::GetStats("/proc/" + std::to_string(e) +
//           "/stat");
//       measurements.processes.push_back(processMeasurement);
//     }
//     catch (const std::exception &error)
//     {
//       CLogger::Log(CLogger::Types::INFO, "Process: ", e,
//                    " ended, removing from measurements");
//       RemoveProcessId(e); // Process apparently stopped already
//     }
//   }
//   pProcessesData_->push_back(measurements);
// }

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

  // CheckThresholds();
  // Collect all sensors
  auto gstSensors = gstMeasurements_.GetSensors();
  auto sysSensors = sensorMeasurements_.GetSensors();
  auto processSensors = processMeasurements_.GetSensors();
  std::cout << "Process sensors size: " << processSensors.size() << std::endl;
  Exports::AllSensors allSensors;
  allSensors.AddSensors(Measurements::Classification::PIPELINE, gstSensors);
  allSensors.AddSensors(Measurements::Classification::SYSTEM, sysSensors);
  allSensors.AddSensors(Measurements::Classification::PROCESSES,
                        processSensors);

  // Check thresholds
  SetThresholdResults(allSensors);
  // SetThresholdResults(sysSensorsMap);

  // pExportObj_ = std::make_unique<Exports::CExport>(
  //     new Exports::CSummaryGenerator{}, "filename", true);
  Exports::CSummaryGenerator generator;
  // generator.Generate(*pMeasurementsData_,
  // sensorMeasurements_.GetDefinition());
  // generator.GenerateProcesses(*pProcessesData_);

  std::vector<Exports::MeasurementItem> items;
  items.push_back(sensorMeasurements_.GetConfig());
  items.push_back(gstMeasurements_.GetPipelineConfig2());
  items.push_back(processMeasurements_.GetConfig());
  generator.FullExport(items, pMeasurementsData_.get(), allSensors);
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
      // if (e.processId != -1)
      // {
      //   auto sensorMapTemp = allSensors.GetMap(e.processId);
      //   auto sensor = sensorMap.find(e.name);
      //   if (sensor == sensorMap.end())
      //   {
      //     CLogger::Log(CLogger::Types::WARNING, "Threshold name: \"", e.name,
      //                  "\" not found!");
      //     continue; // Couldn't check any thresholds, doesn't exist
      //   }
      //   if (!sensor->second->thresholdExceeded)
      //     sensor->second->thresholdExceeded =
      //         PerformanceHelpers::HandleThreshold(sensor->second, e);
      // }
      // else
      // {
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

// void CPerfMeasurements::CheckTresholds() { for (const) }
#if 0
Extra types that need to be supported:
- FPS: average FPS of all sub-modules
- GStreamer values
- Summarized data of all cpu cores (array) or for a single core (also in an array?)

These come above the:
- Sensors in the config file
- Measured thingies for the running processes

And should include both data from the "xavier_sensors" file as well as the "json_config" file. <- why though?
A structure should contain all the sensors, with a boolean for whether the sensor has exceeded the threshold

The thresholds:
Ideally, use a map: std::unordered_map<std::string, Data> where Data can be a pointer to the data. The string is the "userid", for groups, add them separately
#endif
/*
CMonitoring::SCpuInfo CMonitoring::GetCPUInfo()
{
SCpuInfo cpuInfo;

std::string location = "/proc/" + _applicationPID + "/stat";
std::fstream file(location, std::fstream::in);
if (file.good())
{
  std::string cpuProcess;
  getline(file, cpuProcess);
  std::vector<int> vectored =
      stringToVector(deleteNonNumericAndSpace(cpuProcess));
  if (vectored.size() < 20)
  {
    return {};
  }
  int procTimes = vectored.at(12) + vectored.at(13);
  return procTimes;
}
throw std::runtime_error("Couldn't open file!");
}
CMonitoring::SMemoryInfo CMonitoring::GetMemoryInfo() { return {}; }
CMonitoring::SBandwidth CMonitoring::GetMemoryBandwidth() { return {}; } */

/**
 * \brief temperature returns the temperature of the processor
 * \return a double with the processor temperature in degrees Celsius or -1
 * when an error occurs
 */
/* double CMonitoring::GetCPUTemperature(const std::string &tempLocation)
{
  std::string buffer;
  std::ifstream temp(tempLocation);
  if (!temp.is_open() || !temp.good())
  {
    return tempNotAvailable_;
  }
  temp >> buffer;
  temp.close();

  if (buffer.empty())
    return -1.0;
  return (std::stoi(buffer) / tempToCelsiusDivider_);
}

std::vector<CMonitoring::SCoreTemperature> CMonitoring::GetCpuTemperatures()
{
  std::vector<SCoreTemperature> temperatures;
  for (int i = 0; i < maxCpuCores_; i++)
  {
    std::string temperaturePath =
        std::regex_replace(tempLocation_, std::regex("*"), std::to_string(i));
    auto cpuTemperature = GetCPUTemperature();
    if (cpuTemperature < 0)
      break;
    temperatures.push_back(SCoreTemperature{i, cpuTemperature});
  }
  return temperatures;
} */

// CPerfMeasurements::MeasureCombo CPerfMeasurements::GetFields(
//     std::vector<PlatformConfig::SDatafields> &sensorConfig,
//     const std::function<MeasureCombo(CPerfMeasurements *,
//                                      const PlatformConfig::SDatafields &)>
//         parserFunction,
//     CPerfMeasurements *memberPtr)
// {
//   MeasureCombo result;

//   std::for_each(sensorConfig.begin(), sensorConfig.end(),
//                 [&](const PlatformConfig::SDatafields &dataField) {
//                   result.Add(parserFunction(memberPtr, dataField));
//                 });

//   return result;
// }

// CPerfMeasurements::MeasureCombo CPerfMeasurements::GetMeasureFields(
//     const PlatformConfig::SDatafields &dataField)
// {
//   MeasureCombo result;

//   switch (dataField.type)
//   {
//   case PlatformConfig::Types::ARRAY:
//     result.Add(ParseArray(dataField));
//     break;
//   case PlatformConfig::Types::DIRECT:
//   case PlatformConfig::Types::PROC_STAT:
//   case PlatformConfig::Types::PROC_MEM:
//     result.Add(ParseField(dataField));
//     break;
//   default:;
//   }
//   return result;
// }

// CPerfMeasurements::MeasureCombo CPerfMeasurements::GetProcessFields(
//     const PlatformConfig::SDatafields &dataField)
// {
//   MeasureCombo result;

//   switch (dataField.type)
//   {
//   case PlatformConfig::Types::PID_STAT:
//     result.Add(ParseField(dataField));
//     break;
//   default:;
//   }
//   return {result};
// }

// CPerfMeasurements::MeasureComboSingular
// CPerfMeasurements::ParseField(const PlatformConfig::SDatafields &data)
// {
//   MeasureComboSingular result;
//   result.field.path = data.path;
//   result.field.type = data.type;
//   result.field.id = PerformanceHelpers::GetUniqueId();
//   result.definition = data;
//   result.definition.id = result.field.id;
//   return result;
// }

// CPerfMeasurements::MeasureCombo
// CPerfMeasurements::ParseArray(const PlatformConfig::SDatafields &data)
// {
//   MeasureCombo result;
//   // Loop through the defined array
//   for (size_t i = 0; i < data.size; ++i)
//   {
//     // Adjust all variables that are defined within the array
//     for (const auto &e : data.datafields)
//     {
//       auto datafieldCopy{e};
//       Helpers::replaceStr(datafieldCopy.path, "$INDEX$", std::to_string(i));
//       datafieldCopy.name = datafieldCopy.name + std::to_string(i);
//       result.Add(GetMeasureFields(datafieldCopy));
//     }
//   }
//   return result;
// }

// std::vector<Exports::MeasuredItem>
// CPerfMeasurements::GetMeasuredItems(const MeasureFieldsType &measureFields)
// {
//   procHandler_.ParseProcStat();
//   std::vector<Exports::MeasuredItem> measuredItems;
//   for (const auto &e : measureFields)
//   {
//     switch (e.type)
//     {
//     case PlatformConfig::Types::DIRECT:
//       measuredItems.push_back(
//           CXavierSensors::ParseDirect(e)); // ParseDirect(e);
//       break;
//     case PlatformConfig::Types::PROC_STAT:
//     {
//       auto definition = GetFieldDef(e.id);
//       measuredItems.push_back(procHandler_.ParseProcField(definition,
//       e.path));
//     }
//     break;
//     case PlatformConfig::Types::PROC_MEM:
//     {
//       auto definition = GetFieldDef(e.id);
//       measuredItems.push_back(procHandler_.ParseMemField(definition));
//     }
//     break;
//     default:
//       throw std::runtime_error(
//           "Software Error! Incorrect type in the performance measurements!");
//     }
//   }
//   return measuredItems;
// }

} // namespace Linux