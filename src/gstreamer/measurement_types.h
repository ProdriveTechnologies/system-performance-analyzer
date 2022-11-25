#pragma once

#include <string>

namespace GStreamer
{
enum class EMeasureType : int
{
  NONE = 0,
  FPS = 1,
  LATENCY = 2,
  PROCESSING_TIME = 3
};
inline std::string GetMeasureType(const EMeasureType t)
{
  switch (t)
  {
  case EMeasureType::FPS:
    return "fps";
  case EMeasureType::LATENCY:
    return "latency";
  case EMeasureType::PROCESSING_TIME:
    return "processing time";
  case EMeasureType::NONE:
  default:
    return "";
  }
}

struct EMeasurement
{
  EMeasureType type = EMeasureType::NONE;
  std::string pluginName;
  std::string value;
  unsigned int valueInt = 0;
};
} // namespace GStreamer