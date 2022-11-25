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
struct ProcessInfo
{
  unsigned int processId;
  MeasuredItem measuredItem;
};
struct PipelineInfo
{
  int pipelineId;
  MeasuredItem measuredItem;
};
struct ExportData
{
  std::string time;

  // Collected system-wide information
  std::vector<MeasuredItem> systemInfo;

  // Collected information per process
  std::vector<ProcessInfo> processInfo;

  // Collected information per GStreamer stream
  std::vector<PipelineInfo> pipelineInfo;

  std::vector<double> coreUtilization;
  // std::vector<Process> runningProcessIds;
  double memoryUsage;
  // std::vector<CXavierSensors::SCpuCoreInfo> cpuInfo;
  // Linux::FileSystem::ProcStatData cpuUtilization;
  std::vector<int> pids;
  std::vector<MeasuredItem> measuredItems;
};
} // namespace Exports