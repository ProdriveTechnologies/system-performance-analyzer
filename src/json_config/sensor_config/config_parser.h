#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "config.h"

namespace PlatformConfig
{
SConfig Parse(const std::string &jsonFile);

nlohmann::json getJsonObj(const std::string &jsonFile);

inline void from_json(const nlohmann::json &j, SDatafields &p)
{
  j.at("name").get_to(p.name);
  j.at("id").get_to(p.id);
  j.at("path").get_to(p.path);
  j.at("type").get_to(p.type);
}

inline void from_json(const nlohmann::json &j, SSensors &p)
{
  j.at("name").get_to(p.name);
  j.at("type").get_to(p.type);
  j.at("size").get_to(p.size);
  j.at("data").get_to(p.datafields);
}

inline void from_json(const nlohmann::json &j, SConfig &p)
{
  j.at("name").get_to(p.name);
  j.at("sensors").get_to(p.sensors);
}

} // namespace PlatformConfig