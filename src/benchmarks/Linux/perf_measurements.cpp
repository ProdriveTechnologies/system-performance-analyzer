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

#include "src/helpers/stopwatch.h"
namespace Linux
{
CPerfMeasurements::CPerfMeasurements(Synchronizer *synchronizer)
    : threadSync_{synchronizer}, xavierSensors_{XAVIER_CORES}
{
}

void CPerfMeasurements::Start(const Core::SConfig &config)
{
  using Path = Linux::FileSystem::Path;
  threadSync_->WaitForProcess();
  config_ = config;
  Stopwatch stopwatch;

  InitExports();

  // Thread exists now, store threads that already exist for the monitoring. The
  // newly added ones are from gstreamer and need to be monitored
  std::cerr << "Monitoring thread: " << threadSync_->getThreadId();

  Path processPath;
  processPath.AddItems("proc", getpid(), "task");
  excludedThreads_ = FileSystem::GetFiles(processPath.GetPath());

  std::vector<std::string> monitoredThreads;
  // Sync before start, when synced, will start
  //    directly
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
    exportData.cpuInfo = xavierSensors_.GetCoresInfo();
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
void CPerfMeasurements::InitExports()
{
  if (config_.settings.enableLogs)
  {
    pExportObj_ = std::make_unique<Exports::CExport>(new Exports::CCsv{},
                                                     "csvfile.csv", true);
    pExportObj_->InitExport();
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
  CXavierSensors xavierSensors{8};
  auto coresStats = xavierSensors.GetCoresInfo();
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
    throw std::runtime_error("Monitoring: Path doesn't exist!");
  }
  fileObj >> dataBuffer;
  fileObj.close();

  return dataBuffer;
}

} // namespace Linux