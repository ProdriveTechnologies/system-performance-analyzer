#include "config_parser.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace PlatformConfig
{
SConfig Parse(const std::string &jsonFile)
{
  using json = nlohmann::json;
  json jsonObj = getJsonObj(jsonFile);
  SConfig configuration = jsonObj;
  return configuration;
}

nlohmann::json getJsonObj(const std::string &jsonFile)
{
  using json = nlohmann::json;
  std::ifstream jsonFileObj(jsonFile);
  json jsonObj;
  jsonFileObj >> jsonObj;
  return jsonObj;
}

} // namespace PlatformConfig