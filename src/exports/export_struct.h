#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "src/benchmarks/gstreamer_identifier.h"
#include "src/json_config/sensor_config/config.h"
//#include "src/benchmarks/linux/xavier_sensors.h"
#include "src/benchmarks/linux/struct_sensors.h"
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
struct PipelineInfo
{
  int pipelineId;
  std::vector<MeasuredItem> measuredItems;
};
using ProcessInfo = PipelineInfo;
struct PipelineConfig
{
  size_t pipelineId;
  std::string pipelineCommand;
  std::unordered_map<int, GStreamer::Identifier> pluginNames;
};
enum class Type
{
  MEASUREMENT,
  INFO,
  LABEL,
  ARRAY
};
struct MeasurementItem
{
  using MeasurementItems = std::vector<MeasurementItem>;
  std::string name;
  Type type;
  std::variant<MeasurementItems, std::string, int, double> value;
};
struct ExportConfig
{
  std::vector<PlatformConfig::SDatafields> measurementsConfig;

  // Only available after a test
  std::vector<PipelineConfig> pipelineConfig;
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
  // std::vector<CXavierSensors::SCpuCoreInfo> cpuInfo;
  // Linux::FileSystem::ProcStatData cpuUtilization;
  std::vector<MeasuredItem> measuredItems;

  std::vector<Measurements::Sensors> sensors;
};
} // namespace Exports