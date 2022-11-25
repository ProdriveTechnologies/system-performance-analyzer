#pragma once

// #include "src/benchmarks/Monitoring.h"

#include <string>
#include <vector>

class Synchronizer; // pre-definition

namespace Linux
{
class CMonitoring
{
public:
  struct SCoreTemperature
  {
    int coreId;
    double temperature;
  };

  CMonitoring(Synchronizer *synchronizer);
  void start(const int threadId, const bool *runningPtr);

  // SCpuInfo GetCPUInfo();
  // SMemoryInfo GetMemoryInfo();
  // SBandwidth GetMemoryBandwidth();
  // SCoreTemperature GetTemperatures();

  static std::string readLocation(const std::string &path);

private:
  Synchronizer *threadSync_;

  std::vector<std::string> excludedThreads_;

  void measureThread(const std::string &threadProcLoc);

  // static constexpr double tempNotAvailable_ = -1.0;
  // static constexpr double tempUnreadable = -2.0;
  // std::vector<SCoreTemperature> GetCpuTemperatures();
  // double GetCPUTemperature(const std::string &tempLocation);
  // static constexpr char tempLocation_[] =
  //     "/sys/class/thermal/thermal_zone*/temp";
  // static constexpr double tempToCelsiusDivider_ = 1000.0;
  // static constexpr int maxCpuCores_ = 64;
};

} // namespace Linux