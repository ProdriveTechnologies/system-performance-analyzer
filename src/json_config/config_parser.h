#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

#include "config.h"
#include "src/helpers/json_helpers.h"

namespace Module
{
} // namespace Module

namespace Core
{
class ConfigParser
{
public:
  static SConfig Parse(const std::string &jsonFile);

private:
  static nlohmann::json getJsonObj(const std::string &jsonFile);
};

//*********** Conversions for the Core structs to JSON objects ***************//
template <typename ConfigType>
inline void from_json(const nlohmann::json &j, SConfigParam<ConfigType> &p)
{
  j.at("name").get_to(p.name);
  j.at("unit").get_to(p.unit);
  j.at("value").get_to(p.value);
}
inline void from_json(const nlohmann::json &j, SBenchmarkConfig &p)
{
  j.at("min_latency").get_to(p.minLatency);
  j.at("max_latency").get_to(p.maxLatency);
  j.at("max_latency_variation").get_to(p.maxLatencyVariation);
  if (j.contains("custom_params_float"))
    j.at("custom_params_float").get_to(p.customParamsF);
  if (j.contains("custom_params_int"))
    j.at("custom_params_int").get_to(p.customParamsI);
}

inline void from_json(const nlohmann::json &j, STask &p)
{
  j.at("name").get_to(p.name);
  j.at("mode").get_to(p.mode);
  j.at("type").get_to(p.type);
  j.at("task_id").get_to(p.taskId);
  j.at("in").get_to(p.inIds);
  j.at("out").get_to(p.outIds);
  j.at("custom_params").get_to(p.customParams);
  j.at("measurements").get_to(p.benchmarks);
}

inline void from_json(const nlohmann::json &j, SConfig &p)
{
  j.at("name").get_to(p.name);
  j.at("description").get_to(p.description);
  j.at("id").get_to(p.id);
  j.at("version").get_to(p.version);
  j.at("tasks").get_to(p.tasks);
  j.at("gstreamer_pipeline").get_to(p.gstreamerPipeline);
}

} // namespace Core