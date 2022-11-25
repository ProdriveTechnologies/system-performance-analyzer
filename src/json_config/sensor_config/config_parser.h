#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "config.h"

namespace PlatformConfig
{
SConfig Parse(const std::string &jsonFile);

inline void from_json(const nlohmann::json &j, SDatafields &p)
{
  j.at("name").get_to(p.name);
  std::string typeStr;
  j.at("type").get_to(typeStr);
  p.type = GetType(typeStr);
  if (j.contains("id"))
    j.at("id").get_to(p.id);
  if (j.contains("size"))
    j.at("size").get_to(p.size);
  if (j.contains("path"))
  {
    j.at("path").get_to(p.path);
  }
  if (j.contains("class"))
  {
    int classNr;
    j.at("class").get_to(classNr);
    p.classType = GetClass(classNr);
  }
  if (j.contains("value"))
    j.at("value").get_to(p.value);
  if (j.contains("compared_to"))
    j.at("compared_to").get_to(p.comparedTo);
  if (j.contains("data"))
    j.at("data").get_to(p.datafields);
  if (j.contains("suffix"))
    j.at("suffix").get_to(p.suffix);
  if (j.contains("calculation"))
    j.at("calculation").get_to(p.calculation);
}

inline void from_json(const nlohmann::json &j, SConfig &p)
{
  j.at("name").get_to(p.name);
  j.at("sensors").get_to(p.sensors);
}

} // namespace PlatformConfig