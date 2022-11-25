#pragma once

#include <unordered_map>
#include <utility>

#include "src/benchmarks/Linux/xavier_sensors.h"
#include "src/helpers/helper_functions.h"
#include "src/linux/filesystem.h"
#include "src/linux/path_parser_base.h"

namespace Linux
{
class CPidStatmHandler : public CPathParserBase
{
public:
  CPidStatmHandler() = default;

  /**
   * @brief CPidStatHandler has nothing to initialize
   */
  void Initialize() {}
  bool InitializeRuntime(const std::string &replacement)
  {
    std::string path = pidStatmPath;
    Helpers::replaceStr(path, "$PID$", replacement);
    Linux::FileSystem::Statm pidStatmData;
    auto isSuccesful = pidStatmData.ParseStatm(path);
    if (!isSuccesful)
      return false;
    pidStatmData_.insert(std::make_pair(replacement, pidStatmData));
    return true;
  }

  bool ParseMeasurement(const PlatformConfig::SDatafields &datafield,
                        [[maybe_unused]] const std::string &path,
                        const std::string &replacement = "")
  {
    item_.id = datafield.id;

    auto pidStat = pidStatmData_.find(replacement);
    if (pidStat == pidStatmData_.end())
      throw std::runtime_error("CPidStatmHandler: /proc/" + replacement +
                               "/stat not found!");

    item_.measuredValue =
        static_cast<double>(pidStat->second.GetValue(datafield.value));
    if (!datafield.comparedTo.empty())
    {
      unsigned long long comparedTo =
          pidStat->second.GetValue(datafield.comparedTo);
      if (comparedTo != 0)
        item_.measuredValue = item_.measuredValue / comparedTo;
    }

    return true;
  }

private:
  static inline const char pidStatmPath[] = "/proc/$PID$/statm";
  std::unordered_map<std::string, Linux::FileSystem::Statm> pidStatmData_;
};
} // namespace Linux