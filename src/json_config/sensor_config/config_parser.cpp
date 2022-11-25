#include "config_parser.h"

#include "src/helpers/json_helpers.h"

namespace PlatformConfig
{
SConfig Parse(const std::string &jsonFile)
{
  using json = nlohmann::json;
  json jsonObj = JsonHelpers::GetJsonObj(jsonFile);
  SConfig configuration = jsonObj;
  return configuration;
}

} // namespace PlatformConfig