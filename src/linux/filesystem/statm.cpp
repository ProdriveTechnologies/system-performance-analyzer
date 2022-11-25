#include "statm.h"

#include <fstream>
#include <stdexcept>

namespace Linux
{
namespace FileSystem
{
bool Statm::ParseStatm(const std::string &path)
{
  {
    std::ifstream statmFile{path, std::ios_base::in};
    if (!statmFile.good())
    {
      return false;
    }
    std::vector<std::string> statmElements;

    for (std::string val; std::getline(statmFile, val, ' ');)
      statmElements.push_back(val);

    if (statmElements.back().back() == '\n')
      statmElements.back().pop_back();

    values_.clear();
    for (size_t i = 0; i < statmElements.size(); ++i)
    {
      const auto &element = statmElements.at(i);
      const auto &name = orderedNames_.at(i);
      values_.insert(std::make_pair(name, std::stoull(element)));
    }
    return true;
  }
}

unsigned long long Statm::GetValue(const std::string &value)
{
  auto result = values_.find(value);
  if (result == values_.end())
    throw std::runtime_error("Statm: Couldn't find value \"" + value + "\"!");
  return result->second;
}

} // namespace FileSystem
} // namespace Linux