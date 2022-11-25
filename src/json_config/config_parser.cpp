#include "config_parser.h"

#include "src/helpers/json_helpers.h"

namespace Core
{
SConfig ConfigParser::Parse(const std::string &jsonFile)
{
  using json = nlohmann::json;
  json jsonObj = JsonHelpers::GetJsonObj(jsonFile);
  SConfig configuration = jsonObj;
  return configuration;
}

} // namespace Core