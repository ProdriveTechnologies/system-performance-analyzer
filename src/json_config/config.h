#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace Core
{
template <typename ValueType> struct SConfigParam
{
  std::string name;
  std::string unit;
  ValueType value;
};

struct SBenchmarkConfig
{
  int minLatency;
  int maxLatency;
  int maxLatencyVariation;
  std::vector<SConfigParam<float>> customParamsF;
  std::vector<SConfigParam<int>> customParamsI;
};
struct STask
{
  std::string name;
  int taskId;
  std::string type;
  std::string mode;
  std::vector<int> inIds;
  std::vector<int> outIds;
  std::vector<SConfigParam<std::string>> customParams;
  SBenchmarkConfig benchmarks;
  std::string getValWithName(const std::string &name_) const
  {
    for (const auto &e : customParams)
    {
      if (name_ == e.name)
        return e.value;
    }
    throw std::runtime_error("Name not existent");
  }
  bool valExists(const std::string &name_) const
  {
    for (const auto &e : customParams)
    {
      if (name_ == e.name)
        return true;
    }
    return false;
  }
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
  std::string gstreamerPipeline;
  std::vector<STask> tasks;
};

} // namespace Core