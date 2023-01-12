#pragma once

#include "src/json_config/sensor_config/config.h"
#include "src/linux/path_parser_base.h"

#include <string>

namespace Linux
{
class CDirectHandler : public CPathParserBase
{
public:
  CDirectHandler() = default;

  /**
   * @brief CDirectHandler has nothing to initialize
   */
  void Initialize() {}
  bool InitializeRuntime([[maybe_unused]] const std::string& replacement = "") { return true; }

  bool ParseMeasurement(const PlatformConfig::SDatafields& datafield,
                        const std::string& path,
                        [[maybe_unused]] const std::string& replacement = "")
  {
    if (!datafield.enabledPath.empty())
    {
      auto enabledCheck =
        ParseDirect(PlatformConfig::SMeasureField{ datafield.id, datafield.enabledPath, datafield.type });
      if (enabledCheck.measuredValue == DISABLED)
      {
        // Disabled, thus set on 0
        item_ = Measurements::SMeasuredItem{ datafield.id, 0.0 };
        return true;
      }
    }
    item_ = ParseDirect(PlatformConfig::SMeasureField{ datafield.id, path, datafield.type });
    if (item_.id == datafield.id)
      return true;
    errorMsg_ = "Handler DIRECT: Couldn't find correct ID for datafield: " + datafield.name + " and path: " + path;
    return false;
  }

private:
  static constexpr int DISABLED = 0;

  static Measurements::SMeasuredItem ParseDirect(const PlatformConfig::SMeasureField& datafield);
  static std::string ReadLocation(const std::string& path);
};
} // namespace Linux