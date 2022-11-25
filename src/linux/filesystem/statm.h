#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace Linux
{
namespace FileSystem
{
class Statm
{
public:
  bool ParseStatm(const std::string& path);
  unsigned long long GetValue(const std::string& value);

private:
  using ValueName = std::string;
  std::unordered_map<ValueName, unsigned long long> values_;
  inline static const std::vector<ValueName> orderedNames_ = {
    "size", "resident", "shared", "text", "lib", "data", "dt"
  };
};

} // namespace FileSystem

} // namespace Linux