#pragma once

#include <string>
#include <vector>

#include "src/benchmarks/Linux/xavier_sensors.h"

namespace Exports
{
struct Process
{
  int pid;
  std::string appName;
};
struct ExportData
{
  std::string time;
  std::vector<double> coreUtilization;
  std::vector<Process> runningProcessIds;
  double memoryUsage;
  std::vector<CXavierSensors::SCpuCoreInfo> cpuInfo;
};
} // namespace Exports