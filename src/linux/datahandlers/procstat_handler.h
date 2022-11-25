#pragma once

#include "src/benchmarks/linux/proc_handler.h"
#include "src/helpers/helper_functions.h"
#include "src/linux/filesystem.h"
#include "src/linux/path_parser_base.h"

#include <unordered_map>
#include <utility>

namespace Linux
{
class CProcStatHandler : public CPathParserBase
{
public:
  CProcStatHandler() = default;

  /**
   * @brief CProcStatHandler has nothing to initialize except for runtime
   */
  void Initialize() { procHandler_.ParseProcStat(); }

  bool InitializeRuntime([[maybe_unused]] const std::string& replacement)
  {
    procHandler_.ParseProcStat();
    return true;
  }

  bool ParseMeasurement(const PlatformConfig::SDatafields& datafield,
                        [[maybe_unused]] const std::string& path,
                        [[maybe_unused]] const std::string& replacement = "")
  {
    item_ = procHandler_.ParseProcField(datafield, path);
    return true;
  }

private:
  // static inline const char procStatPath_[] = "/proc/stat";
  Measurements::ProcHandler procHandler_;
};
} // namespace Linux