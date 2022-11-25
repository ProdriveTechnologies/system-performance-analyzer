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

ProcStatData GetProcStat()
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

  // const auto &procStatRow = statElements.at(0);
  ProcStatData procStatData;
  // procStatData.totalCpu = ProcStatData::Cpu{procStatRow};

  for (const auto &procStatRow : statElements)
  {
    try
    {
      auto row = std::make_pair(procStatRow.rowElements.at(0),
                                ProcStatData::Cpu{procStatRow});
      procStatData.cpus.insert(row);
    }
    catch (const std::exception &e)
    {
      return procStatData;
    }
  }
  return procStatData;
}

long long GetProcStatGroup(const ProcStatData::Cpu &cpuField,
                           const std::string &groupName)
{
  switch (Helpers::hash(groupName))
  {
  case Helpers::hash("all"):
    return cpuField.jiffiesIdle + cpuField.jiffiesIoWait + cpuField.jiffiesIrq +
           cpuField.jiffiesNice + cpuField.jiffiesSoftIrq +
           cpuField.jiffiesSystem + cpuField.jiffiesUser;
  case Helpers::hash("all_excl_idle"):
    return cpuField.jiffiesIoWait + cpuField.jiffiesIrq + cpuField.jiffiesNice +
           cpuField.jiffiesSoftIrq + cpuField.jiffiesSystem +
           cpuField.jiffiesUser;
  case Helpers::hash("user"):
    return cpuField.jiffiesUser;
  case Helpers::hash("nice"):
    return cpuField.jiffiesNice;
  case Helpers::hash("system"):
    return cpuField.jiffiesSystem;
  case Helpers::hash("idle"):
    return cpuField.jiffiesIdle;
  case Helpers::hash("iowait"):
    return cpuField.jiffiesIoWait;
  case Helpers::hash("irq"):
    return cpuField.jiffiesIrq;
  case Helpers::hash("softirq"):
    return cpuField.jiffiesSoftIrq;

    // case Helpers::hash("steal"):
    // case Helpers::hash("guest"):
    // case Helpers::hash("guest_nice"):
  }
  throw std::runtime_error("Name: " + groupName + "not recognised!");
}
} // namespace FileSystem
} // namespace Linux