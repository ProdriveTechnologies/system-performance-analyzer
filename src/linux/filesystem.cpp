#include "filesystem.h"

#include <filesystem>

namespace Linux
{
namespace FileSystem
{
std::vector<std::string> GetFiles(const std::string &path)
{
  std::vector<std::string> dirFiles;
  for (const auto &entry : std::filesystem::directory_iterator(path))
    dirFiles.push_back(entry.path());
  return dirFiles;
}

Stat GetStats(const std::string &statLocation)
{
  std::ifstream statFile{statLocation, std::ios_base::in};
  if (!statFile.good())
  {
    throw std::runtime_error("Linux::FileSystem: stat file not existent!");
  }
  std::vector<std::string> statElements;
  for (std::string val; std::getline(statFile, val, ' ');)
    statElements.push_back(val);

  if (statElements.back().back() == '\n')
    statElements.back().pop_back();

  return Stat{statElements};
}

} // namespace FileSystem
} // namespace Linux