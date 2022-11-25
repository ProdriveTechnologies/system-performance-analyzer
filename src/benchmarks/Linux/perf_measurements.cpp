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

#include "proc_handler.h"
#include "src/globals.h"
#include "src/helpers/logger.h"
#include "src/helpers/stopwatch.h"
#include "src/json_config/sensor_config/config_parser.h"
// #include "xavier_sensors_live.h"

#include "performance_helpers.h"
#include "src/stdout_export/summary_generator.h"

namespace Linux
{
// TODO: Make cpuUtilizationTimer_ configurable through the JSON
CPerfMeasurements::CPerfMeasurements(Synchronizer *synchronizer,
                                     const std::string &sensorConfig)
    : threadSync_{synchronizer}, sensorConfigFile_{sensorConfig},
      cpuUtilizationTimer_{std::chrono::milliseconds{1000}} //,
// liveFilesystemData_{std::chrono::milliseconds{1000}, XAVIER_CORES}
{
}

/**
 * @brief Will initiate and start the measurements loop
 *
 * @param config
 */
void CPerfMeasurements::Start(const Core::SConfig &config)
{
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
  auto parsed = PlatformConfig::Parse(sensorConfigFile_);
  auto measureFields = GetMeasureFields(parsed.sensors);
  measureFields_ = measureFields.fields;
  measureFieldsDefinition_ = measureFields.definition;
  InitExports(measureFieldsDefinition_);

  pMeasurementsData_ = std::make_unique<std::vector<Exports::ExportData>>();
  pCpuData_ = std::make_unique<std::vector<Linux::FileSystem::ProcStatData>>();
  // Thread exists now, store threads that already exist for the monitoring. The
  // newly added ones are from gstreamer and need to be monitored
  // std::cerr << "Monitoring thread: " << threadSync_->getThreadId();

  // Path processPath;
  // processPath.AddItems("proc", getpid(), "task");
  // excludedThreads_ = FileSystem::GetFiles(processPath.GetPath());

  // Initialise proc stat
  procHandler_.ParseProcStat();
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
    procHandler_.ParseMeminfo(); // Parse the /proc/meminfo struct
    Exports::ExportData exportData;
    exportData.time = std::to_string(
        testRunningTimer_.GetTime<std::milli>()); // Millisecond accuracy
    exportData.measuredItems = GetMeasuredItems(measureFields_);

    pMeasurementsData_->push_back(exportData);
    pCpuData_->push_back(procHandler_.GetStats());

    // exportData.cpuInfo = xavierSensors_.GetCoresInfo();
    // exportData.cpuUtilization = Linux::FileSystem::GetProcStat(XAVIER_CORES);

    // exportData.cpuUtilization = liveFilesystemData_.getLastData();
    // if (cpuUtilizationTimer_.elapsed())
    // {
    //   auto previousAggregated = lastCpuDataAggregated_;
    //   lastCpuDataAggregated_ = Linux::FileSystem::GetProcStat(XAVIER_CORES);
    //   cpuUtilizationTimer_.restart();
    //   lastCpuData_ = lastCpuDataAggregated_ - previousAggregated;
    //   exportData.cpuUtilization = lastCpuData_;
    // }
    // else
    // {
    //   lastCpuData_;
    // }

    // SendExportsData(exportData);

    std::this_thread::sleep_for(
        std::chrono::milliseconds(config_.settings.measureLoopMs));
  }
}

/**
 * @brief initializes the resources used for the measurements
 */
void CPerfMeasurements::InitExports(const MeasureFieldsDefType &config)
{
  if (config_.settings.enableLogs)
  {
    pExportObj_ = std::make_unique<Exports::CExport>(new Exports::CCsv{},
                                                     "csvfile.csv", true);
    pExportObj_->InitExport(config);
  }
}

/**
 * @brief sends the exports data to the export objects
 *
 * @param data the measurements data
 */
void CPerfMeasurements::SendExportsData(const Exports::ExportData &data)
{
  pExportObj_->DataExport(data);
}

void CPerfMeasurements::MeasureThread(const std::string &threadProcLoc)
{
  auto stats = Linux::FileSystem::GetStats(threadProcLoc + "/stat");
  std::cout << "CPU usage: " << stats.cutime << std::endl;
  std::cout << "Minor Faults: " << stats.minFaults << std::endl;
}

void CPerfMeasurements::ExportData()
{
  for (const auto &e : *pMeasurementsData_)
  {
    SendExportsData(e);
  }
}

void CPerfMeasurements::AnalyzeData()
{
  Exports::CSummaryGenerator generator;
  generator.Generate(*pMeasurementsData_, measureFieldsDefinition_);
  //   std::cout << "SPA Tool Summary Report\n" << std::endl;
  //   // First, print the system's specifications
  //   std::cout << "Version\t\t : " << Globals::version << std::endl;
  //   std::cout << "Created\t\t : " << Globals::compilationDate << " "
  //             << Globals::compilationTime << std::endl;
  //   std::cout << std::endl;
  //   std::cout << "Processor and System Information" << std::endl;
  //   const auto quantities = iware::cpu::quantities();
  //   std::cout << "Physical Cores\t\t : " << quantities.physical << std::endl;
  //   std::cout << "Logical Cores\t\t : " << quantities.logical << std::endl;
  //   std::cout << "CPU Package\t\t : " << quantities.packages << std::endl;

  //   for (unsigned i = 1; i <= 3; ++i)
  //   {
  //     const auto cache = iware::cpu::cache(i);
  //     std::cout << "    L" << i << ":\n"
  //               << "      Size         : " << cache.size << "B\n"
  //               << "      Line size    : " << cache.line_size << "B\n"
  //               << "      Associativity: "
  //               << static_cast<unsigned int>(cache.associativity) <<
  //               std::endl;
  //   }
  // Second, summarize system wide data
  // Then, summarize each process the data
  // Finish with the data of each GStreamer pipeline (FPS, most delaying module,
  // CPU heavy module?)
}
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

/**
 * @brief ReadLocation reads the data at a certain path location on Linux
 *
 * @param path the path of the file
 * @return std::string the content of the file
 */
std::string CPerfMeasurements::ReadLocation(const std::string &path)
{
  std::string dataBuffer;
  std::ifstream fileObj(path);
  if (!fileObj.is_open() || !fileObj.good())
  {
    throw std::runtime_error("Monitoring: Path doesn't exist! Path: " + path);
  }
  fileObj >> dataBuffer;
  fileObj.close();

  return dataBuffer;
}

CPerfMeasurements::MeasureCombo CPerfMeasurements::GetMeasureFields(
    const std::vector<PlatformConfig::SDatafields> &sensorConfig)
{
  MeasureCombo result;

  std::for_each(sensorConfig.begin(), sensorConfig.end(),
                [&](const PlatformConfig::SDatafields &dataField) {
                  result.Add(GetMeasureFields(dataField));
                });

  return result;
}

CPerfMeasurements::MeasureCombo CPerfMeasurements::GetMeasureFields(
    const PlatformConfig::SDatafields &dataField)
{
  MeasureCombo result;

  switch (dataField.type)
  {
  case PlatformConfig::Types::ARRAY:
    result.Add(ParseArray(dataField));
    break;
  case PlatformConfig::Types::DIRECT:
  case PlatformConfig::Types::PROC_STAT:
  case PlatformConfig::Types::PROC_MEM:
  // Fallthrough to the "default" case
  default: // Just add the field as-is, for example for the PROC field
    result.Add(ParseField(dataField));
    // throw std::runtime_error(
    //   "Field not found in the sensor file: " +
    //   std::to_string(static_cast<unsigned>(dataField.type)));
  }

  return result;
}

CPerfMeasurements::MeasureComboSingular
CPerfMeasurements::ParseField(const PlatformConfig::SDatafields &data)
{
  MeasureComboSingular result;
  result.field.path = data.path;
  result.field.type = data.type;
  result.field.id = PerformanceHelpers::GetUniqueId();
  result.definition = data;
  result.definition.id = result.field.id;
  return result;
}

CPerfMeasurements::MeasureCombo
CPerfMeasurements::ParseArray(const PlatformConfig::SDatafields &data)
{
  MeasureCombo result;
  // Loop through the defined array
  for (size_t i = 0; i < data.size; ++i)
  {
    // Adjust all variables that are defined within the array
    for (const auto &e : data.datafields)
    {
      auto datafieldCopy{e};
      Helpers::replaceStr(datafieldCopy.path, "$INDEX$", std::to_string(i));
      datafieldCopy.name = datafieldCopy.name + std::to_string(i);
      result.Add(GetMeasureFields(datafieldCopy));
    }
  }
  return result;
}

std::vector<Exports::MeasuredItem>
CPerfMeasurements::GetMeasuredItems(const MeasureFieldsType &measureFields)
{
  procHandler_.ParseProcStat();
  std::vector<Exports::MeasuredItem> measuredItems;
  for (const auto &e : measureFields)
  {
    switch (e.type)
    {
    case PlatformConfig::Types::DIRECT:
      measuredItems.push_back(
          CXavierSensors::ParseDirect(e)); // ParseDirect(e);
      break;
    case PlatformConfig::Types::PROC_STAT:
    {
      auto definition = GetFieldDef(e.id);
      measuredItems.push_back(procHandler_.ParseProcField(definition, e.path));
    }
    break;
    case PlatformConfig::Types::PROC_MEM:
    {
      auto definition = GetFieldDef(e.id);
      measuredItems.push_back(procHandler_.ParseMemField(definition));
    }
    break;
    default:
      throw std::runtime_error(
          "Software Error! Incorrect type in the performance measurements!");
    }
  }
  return measuredItems;
}

} // namespace Linux