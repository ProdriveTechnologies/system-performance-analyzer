#pragma once

#include <string>

namespace GStreamer
{
enum class MeasureType : int
{
  NONE = 0,
  FPS = 1,
  LATENCY = 2,
};
inline std::string GetMeasureType(const MeasureType t)
{
  switch (t)
  {
  case MeasureType::FPS:
    return "fps";
  case MeasureType::LATENCY:
    return "latency";
  case MeasureType::NONE:
  default:
    return "";
  }
}

struct Measurement
{
  MeasureType type = MeasureType::NONE;
  std::string pluginName;
  std::string value;
  unsigned int valueInt = -1;
};
} // namespace GStreamer