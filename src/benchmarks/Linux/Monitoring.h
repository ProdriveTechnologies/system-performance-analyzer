#pragma once

#include "src/Benchmarking/Monitoring.h"

namespace Linux
{
class CMonitoring : public IMonitoring
{
public:
  struct SCoreTemperature
  {
    int coreId;
    double temperature;
  }

  SCpuInfo
  GetCPUInfo();
  SMemoryInfo GetMemoryInfo();
  SBandwidth GetMemoryBandwidth();

private:
  static constexpr double tempNotAvailable_ = -1.0;
  static constexpr double tempUnreadable = -2.0;
  std::vector<SCoreTemperature> GetCpuTemperatures();
  double GetCPUTemperature();
  static constexpr char tempLocation_ = "/sys/class/thermal/thermal_zone*/temp";
  static constexpr double tempToCelsiusDivider_ = 1000.0;
  static int maxCpuCores_ = 64;
};

} // namespace Linux