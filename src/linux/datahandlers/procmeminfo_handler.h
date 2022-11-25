#pragma once

#include <unordered_map>
#include <utility>

#include "src/benchmarks/linux/proc_handler.h"
#include "src/helpers/helper_functions.h"
#include "src/linux/filesystem.h"
#include "src/linux/path_parser_base.h"

namespace Linux
{
class CProcMeminfoHandler : public CPathParserBase
{
public:
  CProcMeminfoHandler() = default;

  /**
   * @brief CProcMeminfoHandler has nothing to initialize except for runtime
   */
  void Initialize() { procHandler_.ParseMeminfo(); }

  bool InitializeRuntime([[maybe_unused]] const std::string &replacement)
  {
    procHandler_.ParseMeminfo();
    return true;
  }

  bool ParseMeasurement(const PlatformConfig::SDatafields &datafield,
                        [[maybe_unused]] const std::string &path,
                        [[maybe_unused]] const std::string &replacement = "")
  {
    item_ = procHandler_.ParseMemField(datafield);
    return true;
  }

private:
  // static inline const char procStatPath_[] = "/proc/stat";
  Measurements::ProcHandler procHandler_;
};
} // namespace Linux