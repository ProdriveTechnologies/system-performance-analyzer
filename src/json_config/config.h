#pragma once

#include "src/helpers/helper_functions.h"
#include "src/helpers/logger.h"
#include <stdexcept>
#include <string>
#include <vector>

#include "src/exports/export_types.h"

namespace Core
{
struct SProcess
{
  int processId = -1;
  std::string type;
  std::string command;
  int startDelay = 0;
  bool useSteadyState = false;
};
enum class ThresholdType
{
  MIN,
  MAX,
  AVERAGE,
  MEDIAN
};
enum class Sign
{
  LT, // Less than
  GT, // Greater than
  LE, // Less equal
  GE  // Greater equal
};
inline Sign GetSign(const std::string &sign)
{
  if (sign == ">")
    return Sign::GT;
  else if (sign == "<")
    return Sign::LT;
  else if (sign == ">=")
    return Sign::GE;
  else if (sign == "<=")
    return Sign::LE;
  throw std::runtime_error("Could not recognise sign \"" + sign + "\"");
}
struct SThreshold
{
  std::string name;
  ThresholdType type;
  Sign sign;
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
  bool verbose;
  bool verboseSummary = false;
  bool enableLogs;
  int measureLoopMs;
  std::vector<SExports> exports;
  bool enableProcTime = false;
  bool enableLiveMode = false;
};
struct SConfig
{
  std::string getStr()
  {
    return "name: " + name + ", description: " + description +
           ", id: " + std::to_string(id) + ", version: " + version;
  }
  std::string name;
  std::string description;
  int id;
  std::string version;
  std::vector<SProcess> processes;
  // std::vector<STask> tasks;
  std::vector<SThreshold> thresholds;
  SSettings settings;

  SProcess GetProcess(const int processId) const
  {
    for (const auto &e : processes)
    {
      if (e.processId == processId)
        return e;
    }
    throw std::runtime_error("Couldn't find processId in the processes!");
  }
};
inline ThresholdType GetThresholdType(const std::string &thresholdGroup)
{
  switch (Helpers::hash(thresholdGroup))
  {
  case Helpers::hash("minimum"):
    return ThresholdType::MIN;
  case Helpers::hash("maximum"):
    return ThresholdType::MAX;
  case Helpers::hash("average"):
    return ThresholdType::AVERAGE;
  case Helpers::hash("median"):
    return ThresholdType::MEDIAN;
  default:
    CLogger::Log(CLogger::Types::ERROR, "Threshold group not recognised!");
    throw std::runtime_error("Threshold group not recognised!");
    break;
  }
}

} // namespace Core