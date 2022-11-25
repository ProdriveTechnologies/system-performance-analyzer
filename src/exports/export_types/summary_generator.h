#pragma once

// #include <infoware/cpu.hpp>
#include <string>

#include "src/benchmarks/linux/perf_measurements.h"
#include "src/exports/export_struct.h"
#include "src/exports/exports_base.h"
#include "src/json_config/sensor_config/config.h"
// #include "src/linux/filesystem.h"

namespace Exports
{
class CSummaryGenerator : public CBase
{
public:
  // bool
  // Generate(const std::vector<Exports::ExportData> &measurementsData,
  //          const std::vector<PlatformConfig::SDatafields> &measurementsDef);
  // bool GenerateProcesses(
  //     const std::vector<Linux::CPerfMeasurements::ProcessesMeasure>
  //         &measuredProcesses);

  bool FullExport(const std::vector<MeasurementItem> &config,
                  const FullMeasurement data, const AllSensors &allSensors,
                  const std::vector<Measurements::CCorrelation::SResult>
                      &correlationResults);

private:
  // Static info based on the host system
  void PrintApplicationInfo(const std::string &totalExecTime);
  void PrintSystemInfo();
  void PrintCacheInfo();
  void PrintPcieInfo();
  void PrintThresholds(const AllSensors &allSensors);
  void PrintCorrelations(const std::vector<Measurements::CCorrelation::SResult>
                             &correlationResults);
  std::string GetTotalTime(const FullMeasurement &fullMeasurement)
  {
    if (fullMeasurement->empty())
      return "0";
    else
      return fullMeasurement->back().time;
  }
  std::string PrintValues(const Measurements::SensorData data)
  {
    std::string result = "\n";
    for (const auto &e : data.summarizedValues)
      result += "\t" + ToString(e.type) + ": " + std::to_string(e.value) + "\n";
    return result;
  }

  double GetAverage(const std::vector<Exports::ExportData> &measurementsData,
                    const int id);

  // Dynamic info based on the test which was executed
  // void PrintSystemSummary(
  //     const std::vector<Exports::ExportData> &measurementsData,
  //     const std::vector<PlatformConfig::SDatafields> &measurementsDef);
  void PrintSystemSummary(const AllSensors &allSensors);
  void PrintValue(const std::string_view translation,
                  const Measurements::Sensors &sensor,
                  const Measurements::ValueTypes valueType);
};

} // namespace Exports