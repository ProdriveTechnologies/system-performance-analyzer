#include "config_parser.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "src/modules/config.h"

namespace Core
{
Config ConfigParser::Parse(const std::string &jsonFile)
{
  using json = nlohmann::json;
  json jsonObj = getJsonObj(jsonFile);
  Config configuration = jsonObj;
  return configuration;
}

nlohmann::json ConfigParser::getJsonObj(const std::string &jsonFile)
{
  using json = nlohmann::json;
  std::ifstream jsonFileObj(jsonFile);
  json jsonObj;
  jsonFileObj >> jsonObj;
  return jsonObj;
}

} // namespace Core