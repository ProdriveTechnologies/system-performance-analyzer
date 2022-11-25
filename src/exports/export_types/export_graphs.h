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
  CGraphs() = default;

  bool FullExport(
      const std::vector<MeasurementItem> &config, const FullMeasurement data,
      const AllSensors &allSensors,
      const std::vector<Measurements::CCorrelation::SResult> &correlations);

private:
  AllSensors allSensors_;

  void CreateGraph(const Measurements::Sensors &sensor1,
                   const Measurements::Sensors &sensor2);

  std::string GetFileName(const Measurements::Classification c);
};

} // namespace Exports