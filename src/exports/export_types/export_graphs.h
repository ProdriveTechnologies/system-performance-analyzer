#pragma once

#include <array>
#include <unordered_map>

#include "../exports_base.h"
#include "src/helpers/helper_functions.h"

namespace Exports
{
class CGraphs : public CBase
{
public:
  bool FullExport(
      const std::vector<MeasurementItem> &config, const FullMeasurement data,
      const AllSensors &allSensors,
      const std::vector<Measurements::CCorrelation::SResult> &correlations);

private:
  void CreateGraph(const Measurements::Sensors &sensor1,
                   const Measurements::Sensors &sensor2);
};

} // namespace Exports