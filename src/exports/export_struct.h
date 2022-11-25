#pragma once

#include <string>
#include <vector>

//#include "src/benchmarks/Linux/xavier_sensors.h"
#include "src/linux/filesystem.h"

namespace Exports
{
struct Process
{
  int pid;
  std::string appName;
};
struct MeasuredItem
{
  int id;
  double measuredValue;
};
struct ExportData
{
  std::string time;
  std::vector<double> coreUtilization;
  // std::vector<Process> runningProcessIds;
  double memoryUsage;
  // std::vector<CXavierSensors::SCpuCoreInfo> cpuInfo;
  // Linux::FileSystem::ProcStatData cpuUtilization;
  std::vector<int> pids;
  std::vector<MeasuredItem> measuredItems;
};
} // namespace Exports