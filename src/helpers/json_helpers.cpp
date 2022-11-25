#include "json_helpers.h"

#include <fstream>
#include <stdexcept>

namespace JsonHelpers
{
nlohmann::json GetJsonObj(const std::string& jsonFile)
{
  using json = nlohmann::json;
  std::string fileContent = ReadFileIntoString(jsonFile);
  json jsonObj = json::parse(fileContent, nullptr, true, true /* ignore comments */);
  return jsonObj;
}

std::string ReadFileIntoString(const std::string& path)
{
  std::ifstream file(path);
  if (!file.is_open())
    throw std::runtime_error("Could not open the file! File: " + path);

  return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}
} // namespace JsonHelpers