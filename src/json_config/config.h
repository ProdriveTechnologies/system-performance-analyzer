#pragma once

#include "src/exports/export_types.h"
#include "src/gstreamer/measurement_types.h"
#include "src/helpers/helper_functions.h"
#include "src/helpers/logger.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace Core
{
enum class ProcessType
{
  GSTREAMER,
  LINUX_PROCESS
};
inline ProcessType GetProcessType(const std::string& processType)
{
  if (processType == "gstreamer")
    return ProcessType::GSTREAMER;
  else if (processType == "linux_command")
    return ProcessType::LINUX_PROCESS;
  throw std::runtime_error("Could not recognise process type \"" + processType +
                           "\", can be: \"gstreamer\" or \"linux_command\"");
}
struct SProcess
{
  int processId = -1;
  ProcessType type = ProcessType::LINUX_PROCESS;
  std::string command;
  int startDelay = 0;
  bool useSteadyState = false;
};
enum class EThresholdType
{
  MIN,
  MAX,
  AVERAGE,
  MEDIAN
};
enum class ESign
{
  LT, // Less than
  GT, // Greater than
  LE, // Less equal
  GE  // Greater equal
};
inline ESign GetSign(const std::string& sign)
{
  if (sign == ">")
    return ESign::GT;
  else if (sign == "<")
    return ESign::LT;
  else if (sign == ">=")
    return ESign::GE;
  else if (sign == "<=")
    return ESign::LE;
  throw std::runtime_error("Could not recognise sign \"" + sign + "\"");
}
struct SThreshold
{
  std::string name;
  EThresholdType type;
  ESign sign;
  int processId;
  double value;
};
struct SExports
{
  Exports::ETypes exportType;
  bool exportEnabled = false;
  std::string filename;
};

struct SSettings
{
  struct SPipelineSensorConfig
  {
    // The minimum and maximum value used for the live performance bar. These are only used for pipeline values, as the
    // sensorfile is not used for them
    int minimumValue = 0;
    int maximumValue = 250;
    GStreamer::EMeasureType sensorType = GStreamer::EMeasureType::NONE;
  };

  bool verbose;
  bool verboseSummary = false;
  bool enableLogs;
  int measureLoopMs;
  std::vector<SExports> exports;
  bool enableProcTime = false;
  bool enableLiveMode = false;
  bool enablePretestZeroes = false; // Used for better bottleneck detection
  std::vector<SPipelineSensorConfig> pipelineConfig;

  SPipelineSensorConfig GetSensorConfig(const GStreamer::EMeasureType sensorType) const
  {
    auto res = std::find_if(pipelineConfig.begin(), pipelineConfig.end(), [sensorType](const auto& e) {
      return e.sensorType == sensorType;
    });
    return res == pipelineConfig.end() ? SPipelineSensorConfig{} : *res;
  }
};
struct SConfig
{
  std::string getStr()
  {
    return "name: " + name + ", description: " + description + ", id: " + std::to_string(id) + ", version: " + version;
  }
  std::string name;
  std::string description;
  int id;
  std::string version;
  std::vector<SProcess> processes;
  std::vector<SThreshold> thresholds;
  SSettings settings;

  SProcess GetProcess(const int processId) const
  {
    for (const auto& e : processes)
    {
      if (e.processId == processId)
        return e;
    }
    throw std::runtime_error("Couldn't find processId in the processes!");
  }
};
inline EThresholdType GetThresholdType(const std::string& thresholdGroup)
{
  switch (Helpers::hash(thresholdGroup))
  {
  case Helpers::hash("minimum"):
    return EThresholdType::MIN;
  case Helpers::hash("maximum"):
    return EThresholdType::MAX;
  case Helpers::hash("average"):
    return EThresholdType::AVERAGE;
  case Helpers::hash("median"):
    return EThresholdType::MEDIAN;
  default:
    CLogger::Log(CLogger::Types::ERROR, "Threshold group not recognised!");
    throw std::runtime_error("Threshold group not recognised!");
    break;
  }
}

} // namespace Core