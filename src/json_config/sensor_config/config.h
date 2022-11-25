#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace PlatformConfig
{
struct SDatafields
{
  std::string name;
  int id;
  std::string type;

  // Required for type: PROC or DIRECT
  std::string pathStr;
  std::string path;

  // Required for type: PROC
  std::string field;
  std::string value;
  std::string comparedTo;

  // Required for type: ARRAY
  size_t size;
  std::vector<SDatafields> datafields;
};
struct SConfig
{
  std::string name;
  std::vector<SDatafields> sensors;
};

} // namespace PlatformConfig