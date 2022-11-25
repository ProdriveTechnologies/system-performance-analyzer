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
      const std::vector<SMeasurementItem> &config, const FullMeasurement data,
      const AllSensors &allSensors,
      const std::vector<Measurements::CCorrelation::SResult> &correlations);

private:
  AllSensors allSensors_;

  void CreateGraph(const Measurements::SSensors &sensor1,
                   const Measurements::SSensors &sensor2);

  std::string GetFileName(const Measurements::EClassification c);
};

} // namespace Exports