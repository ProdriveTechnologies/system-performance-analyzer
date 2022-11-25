#pragma once

#include "../exports_base.h"
#include "src/helpers/helper_functions.h"

#include <array>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

namespace Exports
{
class CJson : public CBase
{
public:
  bool FullExport(const std::vector<SMeasurementItem>& config,
                  const FullMeasurement data,
                  const AllSensors& allSensors,
                  const std::vector<Measurements::CCorrelation::SResult>& correlations) override;

private:
  static constexpr char EXTENSION[] = ".json";
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

  struct SCsvStructure
  {
    std::array<std::string, Helpers::ToUnderlying(CsvIndex::MAX_SIZE)> csvRows;
  };
  std::string InitPipelineConfig(const size_t pipelineId, const std::unordered_map<int, GStreamer::SIdentifier>& items);
  nlohmann::json ParseLabel(const SMeasurementItem& item, const nlohmann::json parent = nlohmann::json{});
};

} // namespace Exports