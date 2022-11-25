#pragma once

#include <unordered_map>
#include <utility>

#include "src/benchmarks/Linux/xavier_sensors.h"
#include "src/helpers/helper_functions.h"
#include "src/linux/filesystem.h"
#include "src/linux/path_parser_base.h"

namespace Linux
{
class CPidStatHandler : public CPathParserBase
{
public:
  CPidStatHandler() = default;

  /**
   * @brief CPidStatHandler has nothing to initialize
   */
  void Initialize() {}
  bool InitializeRuntime(const std::string &replacement)
  {
    std::string path = pidStatPath;
    Helpers::replaceStr(path, "$PID$", replacement);
    Linux::FileSystem::Stat pidStatData{Linux::FileSystem::GetStats(path)};
    if (!pidStatData.succesful)
      return false;
    pidStatData_.insert(std::make_pair(replacement, std::move(pidStatData)));
    return true;
  }

  bool ParseMeasurement(const PlatformConfig::SDatafields &datafield,
                        [[maybe_unused]] const std::string &path,
                        const std::string &replacement = "")
  {
    item_.id = datafield.id;

    auto pidStat = pidStatData_.find(replacement);
    if (pidStat == pidStatData_.end())
      throw std::runtime_error("CPidStatHandler: /proc/" + replacement +
                               "/stat not found!");

    item_.measuredValue = pidStat->second.GetValue<double>(datafield.value);
    if (!datafield.comparedTo.empty())
    {
      double comparedTo =
          pidStat->second.GetValue<double>(datafield.comparedTo);
      if (comparedTo != 0.0)
        item_.measuredValue = item_.measuredValue / comparedTo;
    }

    return true;
  }

private:
  static inline const char pidStatPath[] = "/proc/$PID$/stat";
  std::unordered_map<std::string, Linux::FileSystem::Stat> pidStatData_;
};
} // namespace Linux