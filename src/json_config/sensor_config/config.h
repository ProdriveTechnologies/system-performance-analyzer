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
  std::string pathStr;
  std::vector<std::string> path;
};
struct SSensors
{
  std::string name;
  int id;
  std::string type;
  size_t size; // optional
  std::vector<SDatafields> datafields;
};
struct SConfig
{
  std::string name;
  std::vector<SSensors> sensors;
};

} // namespace PlatformConfig