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

#include "src/benchmarks/linux/struct_measurements.h"

namespace Exports
{
struct Process
{
  int pid;
  std::string appName;
};
using SMeasuredItem = Measurements::SMeasuredItem;

static inline SMeasuredItem
FindMeasuredItem(const std::vector<SMeasuredItem> &items, const int id)
{
  for (const auto &e : items)
  {
    if (e.id == id)
      return e;
  }
  return {};
}

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

} // namespace Exports