#include "Monitoring.h"

#include <iostream>
#include <regex>       // std::regex_replace
#include <sys/types.h> // getpid()
#include <thread>
#include <unistd.h> // getpid()

#include "Helpers.h"
#include "src/helpers/synchronizer.h"
#include "src/linux/filesystem.h"

namespace Linux
{
CMonitoring::CMonitoring(Synchronizer *synchronizer) : threadSync_{synchronizer}
{
}

void CMonitoring::start(const int threadId, const bool *runningPtr)
{
  using Path = Linux::FileSystem::Path;
  threadSync_->waitForProcess();
  // Thread exists now, store threads that already exist for the monitoring. The
  // newly added ones are from gstreamer and need to be monitored
  std::cerr << "Monitoring thread: " << threadSync_->getThreadId();

  Path processPath;
  processPath.AddItems("proc", getpid(), "task");
  excludedThreads_ = Linux::FileSystem::GetFiles(processPath.GetPath());

  threadSync_
      ->waitForProcess(); // Sync before start, when synced, will start directly
  std::vector<std::string> monitoredThreads;
  while (*runningPtr)
  {
    // 1. get all threads that will be monitored
    monitoredThreads = Linux::FileSystem::GetFiles(processPath.GetPath());
    Helpers::RemoveIntersection(monitoredThreads, excludedThreads_);

    // 2. Loop through threads and execute benchmarks on them
    for (const auto &e : monitoredThreads)
    {
      measureThread(e);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  }
}

void CMonitoring::measureThread(const std::string &threadProcLoc)
{
  auto stats = Linux::FileSystem::GetStats(threadProcLoc + "/stat");
  std::cout << "CPU usage: ";
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
 * @brief readLocation reads the data at a certain path location on Linux
 *
 * @param path the path of the file
 * @return std::string the content of the file
 */
std::string CMonitoring::readLocation(const std::string &path)
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