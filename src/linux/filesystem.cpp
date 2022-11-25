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

ProcStatData GetProcStat(const int cpus)
{
  std::ifstream statFile{"/proc/stat", std::ios_base::in};
  if (!statFile.good())
  {
    throw std::runtime_error("Linux::FileSystem: stat file not existent!");
  }
  std::vector<ProcStatRow> statElements;
  for (std::string val; std::getline(statFile, val);)
  {
    ProcStatRow row{Helpers::Split(val, ' ')};
    if (row.rowElements.back().back() == '\n')
      row.rowElements.back().pop_back();
    statElements.push_back(row);
  }

  ProcStatData procStatData;
  procStatData.totalCpu = ProcStatData::Cpu{statElements.at(0)};
  for (int i = 1; i <= cpus; ++i)
    procStatData.cpus.push_back(ProcStatData::Cpu{statElements.at(i)});

  return procStatData;
}

} // namespace FileSystem
} // namespace Linux