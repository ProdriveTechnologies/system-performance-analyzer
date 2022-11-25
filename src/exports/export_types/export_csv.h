#pragma once

#include "../exports_base.h"
#include "src/helpers/helper_functions.h"

#include <array>
#include <unordered_map>

namespace Exports
{
class CCsv : public CBase
{
public:
  bool FullExport(const std::vector<SMeasurementItem>& config,
                  const FullMeasurement data,
                  const AllSensors& allSensors,
                  const std::vector<Measurements::CCorrelation::SResult>& correlations);

private:
  static constexpr char DELIMITER = ',';

  std::string ParseData(const std::string& timeStr,
                        const std::vector<Measurements::SSensors>& allSensors,
                        const std::vector<SMeasuredItem>& items);
  std::string ParseLabel(const std::vector<Measurements::SSensors>& sensors);
};

} // namespace Exports