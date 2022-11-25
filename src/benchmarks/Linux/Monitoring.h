#pragma once

// #include "src/benchmarks/Monitoring.h"

#include <string>
#include <vector>

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

  // SCpuInfo GetCPUInfo();
  // SMemoryInfo GetMemoryInfo();
  // SBandwidth GetMemoryBandwidth();
  // SCoreTemperature GetTemperatures();

  static std::string readLocation(const std::string &path);

private:
  inline static const std::vector<std::string> locations{
      "/sys/devices/system/cpu/cpu0/online",
      "/sys/devices/system/cpu/cpu1/online",
      "/sys/devices/system/cpu/cpu2/online",
      "/sys/devices/system/cpu/cpu3/online",
      "/sys/devices/system/cpu/cpu4/online",
      "/sys/devices/system/cpu/cpu5/online",
      "/sys/devices/system/cpu/cpu6/online",
      "/sys/devices/system/cpu/cpu7/online"};
  static constexpr double tempNotAvailable_ = -1.0;
  static constexpr double tempUnreadable = -2.0;
  std::vector<SCoreTemperature> GetCpuTemperatures();
  double GetCPUTemperature(const std::string &tempLocation);
  static constexpr char tempLocation_[] =
      "/sys/class/thermal/thermal_zone*/temp";
  static constexpr double tempToCelsiusDivider_ = 1000.0;
  static constexpr int maxCpuCores_ = 64;
};

} // namespace Linux