#pragma once

#include "src/benchmarks/linux/proc_handler.h"
#include "src/linux/path_parser_base.h"

#include <string>

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

  bool InitializeRuntime([[maybe_unused]] const std::string& replacement)
  {
    procHandler_.ParseMeminfo();
    return true;
  }

  bool ParseMeasurement(const PlatformConfig::SDatafields& datafield,
                        [[maybe_unused]] const std::string& path,
                        [[maybe_unused]] const std::string& replacement = "")
  {
    item_ = procHandler_.ParseMemField(datafield);
    return true;
  }

private:
  Measurements::ProcHandler procHandler_;
};
} // namespace Linux