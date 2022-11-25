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
  bool FullExport(
      const std::vector<MeasurementItem> &config, const FullMeasurement data,
      const AllSensors &allSensors,
      const std::vector<Measurements::CCorrelation::SResult> &correlations);

private:
  static constexpr char DELIMITER = ',';

  std::string ParseData(const std::string &timeStr,
                        const std::vector<Measurements::Sensors> &allSensors,
                        const std::vector<SMeasuredItem> &items);
  std::string ParseLabel(const std::vector<Measurements::Sensors> &sensors);
};

} // namespace Exports