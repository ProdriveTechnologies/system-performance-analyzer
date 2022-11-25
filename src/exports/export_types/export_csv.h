#pragma once

#include <array>

#include "../exports_base.h"
#include "src/helpers/helper_functions.h"

namespace Exports
{
class CCsv : public CBase
{
public:
  std::string InitExport(const PlatformConfig::SConfig &config);
  std::string ParseData(const ExportData &data);
  std::string FinishExport();

private:
  enum class CsvIndex : size_t
  {
    TIME = 0,
    CPU_PERCENTAGE_HIGHEST = 1,
    CPU_PERCENTAGE_LOWEST = 2,
    CPU_PERCENTAGE_AVERAGE = 3,
    MEMORY_USAGE = 4,
    TEMPERATURE_HIGHEST = 5,
    TEMPERATURE_AVERAGE = 6,
    MAX_SIZE // Only used for the size of the array
  };
  static constexpr char DELIMITER = ',';

  struct SCsvStructure
  {
    std::array<std::string, Helpers::ToUnderlying(CsvIndex::MAX_SIZE)> csvRows;
  };
};

} // namespace Exports