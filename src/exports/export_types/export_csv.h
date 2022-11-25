#pragma once

#include <array>
#include <unordered_map>

#include "../exports_base.h"
#include "src/helpers/helper_functions.h"

namespace Exports
{
class CCsv : public CBase
{
public:
  std::string ParseData(const std::string &timeStr,
                        const std::vector<Measurements::Sensors> &allSensors,
                        const std::vector<MeasuredItem> &items);
  bool FullExport(
      const std::vector<MeasurementItem> &config, const FullMeasurement data,
      const AllSensors &allSensors,
      const std::vector<Measurements::CCorrelation::SResult> &correlations);

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
  std::string InitPipelineConfig(
      const size_t pipelineId,
      const std::unordered_map<int, GStreamer::Identifier> &items);
  std::string ParseLabel(const std::vector<Measurements::Sensors> &sensors);
};

} // namespace Exports