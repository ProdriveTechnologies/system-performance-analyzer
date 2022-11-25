#pragma once

#include "src/helpers/helper_functions.h"
#include "src/helpers/logger.h"
#include <stdexcept>
#include <string>
#include <vector>

namespace Core
{
// template <typename ValueType> struct SConfigParam
// {
//   std::string name;
//   std::string unit;
//   ValueType value;
// };

// struct SBenchmarkConfig
// {
//   int minLatency;
//   int maxLatency;
//   int maxLatencyVariation;
//   std::vector<SConfigParam<float>> customParamsF;
//   std::vector<SConfigParam<int>> customParamsI;
// };
// struct STask
// {
//   std::string name;
//   int taskId;
//   std::string type;
//   std::string mode;
//   std::vector<int> inIds;
//   std::vector<int> outIds;
//   std::vector<SConfigParam<std::string>> customParams;
//   SBenchmarkConfig benchmarks;
//   std::vector<SThreshold> thresholds;
//   std::string getValWithName(const std::string &name_) const
//   {
//     for (const auto &e : customParams)
//     {
//       if (name_ == e.name)
//         return e.value;
//     }
//     throw std::runtime_error("Name not existent");
//   }
//   bool valExists(const std::string &name_) const
//   {
//     for (const auto &e : customParams)
//     {
//       if (name_ == e.name)
//         return true;
//     }
//     return false;
//   }
// };
struct SProcess
{
  int processId;
  std::string type;
  std::string command;
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