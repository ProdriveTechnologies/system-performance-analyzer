#pragma once

#include "src/helpers/helper_functions.h"

#include <numeric>
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

const inline std::unordered_map<EMeasureType, std::string> typeMapping_{ { EMeasureType::FPS, "fps" },
                                                                         { EMeasureType::LATENCY, "latency" },
                                                                         { EMeasureType::PROCESSING_TIME,
                                                                           "processing time" } };

inline EMeasureType GetMeasureType(const std::string& t)
{
  for (const auto& e : typeMapping_)
  {
    if (e.second == t)
      return e.first;
  }
  std::string err{ "Could not find pipeline measurement type \"" + t + "\"! Possible types: " };
  err = std::accumulate(typeMapping_.begin(), typeMapping_.end(), err, [](const auto& sum, const auto& value) {
    return sum + "\"" + value.second + "\" ";
  });
  throw std::runtime_error(err);
}

inline std::string GetMeasureType(const EMeasureType t)
{
  auto result = typeMapping_.find(t);
  return (result == typeMapping_.end()) ? "" : result->second;
}

struct EMeasurement
{
  EMeasureType type = EMeasureType::NONE;
  std::string pluginName;
  std::string value;
  unsigned int valueInt = 0;
};
} // namespace GStreamer