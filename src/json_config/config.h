#pragma once

#include "src/helpers/helper_functions.h"
#include "src/helpers/logger.h"
#include <stdexcept>
#include <string>
#include <vector>

namespace Core
{
struct SProcess
{
  int processId = -1;
  std::string type;
  std::string command;
  int startDelay = 0;
};
enum class ThresholdType
{
  MIN,
  MAX,
  AVERAGE
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
struct SSettings
{
  bool verbose;
  bool enableLogs;
  int measureLoopMs;
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
  default:
    CLogger::Log(CLogger::Types::ERROR, "Threshold group not recognised!");
    throw std::runtime_error("Threshold group not recognised!");
    break;
  }
}

} // namespace Core