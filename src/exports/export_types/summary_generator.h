#pragma once

// #include <infoware/cpu.hpp>
#include <string>

// #include "src/benchmarks/linux/perf_measurements.h"
#include "src/exports/export_struct.h"
#include "src/exports/exports_base.h"
#include "src/json_config/sensor_config/config.h"
// #include "src/linux/filesystem.h"

namespace Exports
{
class CSummaryGenerator : public CBase
{
public:
  bool FullExport(const std::vector<SMeasurementItem>& config,
                  const FullMeasurement data,
                  const AllSensors& allSensors,
                  const std::vector<Measurements::CCorrelation::SResult>& correlationResults);

private:
  // Static info based on the host system
  void PrintApplicationInfo(const std::string& totalExecTime);
  void PrintThresholds(const AllSensors& allSensors);
  void PrintCorrelations(const std::vector<Measurements::CCorrelation::SResult>& correlationResults);
  std::string GetTotalTime(const FullMeasurement& fullMeasurement)
  {
    if (fullMeasurement->empty())
      return "0";
    else
      return fullMeasurement->back().time;
  }
  std::string PrintValues(const Measurements::SSensorData data)
  {
    std::string result = "\n";
    for (const auto& e : data.summarizedValues)
      result += "\t" + ToString(e.type) + ": " + std::to_string(e.value) + "\n";
    return result;
  }

  // Dynamic info based on the test which was executed
  void PrintSystemSummary(const AllSensors& allSensors);
  void PrintValue(const std::string_view translation,
                  const Measurements::SSensors& sensor,
                  const Measurements::EValueTypes valueType);
};

} // namespace Exports