#include "perf_measurements.h"

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

#include "src/helpers/logger.h"
#include "src/helpers/stopwatch.h"
#include "src/json_config/sensor_config/config_parser.h"
#include "xavier_sensors_live.h"
namespace Linux
{
// TODO: Make cpuUtilizationTimer_ configurable through the JSON
CPerfMeasurements::CPerfMeasurements(Synchronizer *synchronizer)
    : threadSync_{synchronizer}, cpuUtilizationTimer_{std::chrono::milliseconds{
                                     1000}},
      liveFilesystemData_{std::chrono::milliseconds{1000}, XAVIER_CORES}
{
}

void CPerfMeasurements::Start(const Core::SConfig &config)
{
  using Path = Linux::FileSystem::Path;
  CLogger::Log(CLogger::Types::INFO, "Starting synchronize for benchmarks");
  threadSync_->WaitForProcess();
  config_ = config;
  Stopwatch stopwatch;

  auto parsed = PlatformConfig::Parse("xavier_config.json");
  measureFields_ = GetMeasureFields(parsed);
  InitExports(parsed);

  // Thread exists now, store threads that already exist for the monitoring. The
  // newly added ones are from gstreamer and need to be monitored
  std::cerr << "Monitoring thread: " << threadSync_->getThreadId();

  Path processPath;
  processPath.AddItems("proc", getpid(), "task");
  excludedThreads_ = FileSystem::GetFiles(processPath.GetPath());

  std::vector<std::string> monitoredThreads;

  // cpuUtilizationTimer_.restart();
  // lastCpuDataAggregated_ = Linux::FileSystem::GetProcStat(XAVIER_CORES);
  // while (!cpuUtilizationTimer_.elapsed())
  // {
  //   std::this_thread::sleep_for(
  //       std::chrono::milliseconds(cpuUtilizationTimer_.timeTillElapsed()));
  // }
  liveFilesystemData_.Init();

  // Sync before start, when synced, will start directly
  CLogger::Log(CLogger::Types::INFO, "Starting synchronize 2 for benchmarks");
  threadSync_->WaitForProcess();
  stopwatch.Start();
  while (!threadSync_->AllCompleted())
  {
    // 1. get all threads that will be monitored
    monitoredThreads = FileSystem::GetFiles(processPath.GetPath());
    Helpers::RemoveIntersection(monitoredThreads, excludedThreads_);
    // 2. Loop through threads and execute benchmarks on them
    for (const auto &e : monitoredThreads)
    {
      MeasureThread(e);
    }
    // Filling the export data
    Exports::ExportData exportData;
    exportData.time =
        std::to_string(stopwatch.GetTime<std::milli>()); // Millisecond accuracy
    exportData.measuredItems = GetMeasuredItems(measureFields_);
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

    SendExportsData(exportData);

    std::this_thread::sleep_for(
        std::chrono::milliseconds(config_.settings.measureLoopMs));
  }
  // And synchronize the stop, so all threads stop at the same time
  threadSync_->WaitForProcess();
}

/**
 * @brief initializes the resources used for the measurements
 */
void CPerfMeasurements::InitExports(const PlatformConfig::SConfig &config)
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

void CPerfMeasurements::MeasureSystem()
{
  // CXavierSensors xavierSensors{8};
  // auto coresStats = xavierSensors.GetCoresInfo();
  // std::cout << "Some sensor info: " << coresStats.at(0)()
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

std::vector<PlatformConfig::SDatafields>
CPerfMeasurements::GetMeasureFields(const PlatformConfig::SConfig &configFile)
{
  std::vector<PlatformConfig::SDatafields> measureFields;
  for (const auto &e : configFile.sensors)
  {
    switch (Helpers::hash(e.type))
    {
    case Helpers::hash("array"):
      measureFields = Helpers::CombineVectors(measureFields, ParseArray(e));
      break;
    default:
      throw std::runtime_error(
          "Monitoring: unknown type in the configuration file!");
    }
  }
  return measureFields;
}

std::vector<PlatformConfig::SDatafields>
CPerfMeasurements::ParseArray(const PlatformConfig::SSensors &data)
{
  std::vector<PlatformConfig::SDatafields> measureFields;
  for (size_t i = 0; i < data.size; ++i)
  {
    for (const auto &e : data.datafields)
    {
      PlatformConfig::SDatafields dataField{e};
      std::for_each(e.path.begin(), e.path.end(),
                    [&](const std::string &pathElem) {
                      dataField.pathStr +=
                          pathElem == "_INDEX_" ? std::to_string(i) : pathElem;
                    });
      measureFields.push_back(dataField);
    }
  }
  return measureFields;
}

std::vector<Exports::MeasuredItem>
CPerfMeasurements::GetMeasuredItems(const MeasureFieldsType &measureFields)
{
  std::vector<Exports::MeasuredItem> measuredItems;
  for (const auto &e : measureFields)
  {
    switch (Helpers::hash(e.type))
    {
    case Helpers::hash("DIRECT"):
      measuredItems.push_back(
          CXavierSensors::ParseDirect(e)); // ParseDirect(e);
      break;
    }
  }
  return measuredItems;
}

} // namespace Linux