#pragma once

#include "src/benchmarks/linux/xavier_sensors.h"
#include "src/linux/path_parser_base.h"

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
  bool InitializeRuntime([[maybe_unused]] const std::string &replacement = "")
  {
    return true;
  }

  bool ParseMeasurement(const PlatformConfig::SDatafields &datafield,
                        const std::string &path,
                        [[maybe_unused]] const std::string &replacement = "")
  {
    item_ = CXavierSensors::ParseDirect(
        PlatformConfig::SMeasureField{datafield.id, path, datafield.type});
    if (item_.id == datafield.id)
      return true;
    return false;
  }

private:
};
} // namespace Linux