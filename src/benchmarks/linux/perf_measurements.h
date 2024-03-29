#pragma once

#include "src/benchmarks/linux/struct_sensors.h" // SAllSensors
#include "src/benchmarks/pipeline_measurements.h"
#include "src/benchmarks/process_measurements.h"
#include "src/benchmarks/system_measurements.h"
#include "src/exports/export.h" // Measurements::CCorrelation
#include "src/helpers/stopwatch.h"
#include "src/json_config/config.h" // core::SThreshold
#include "src/processes_struct.h"   // ProcessInfo
#include "struct_measurements.h"    // Measurements::SMeasurementsData

#include <string>
#include <vector>

class Synchronizer; // pre-definition

namespace Linux
{
class CPerfMeasurements
{
public:
  struct SCoreTemperature
  {
    int coreId;
    double temperature;
  };

  CPerfMeasurements(Synchronizer* synchronizer,
                    const std::string& sensorConfig,
                    const std::vector<Core::SThreshold>& thresholds);
  void Start(const Core::SConfig& config, std::vector<ProcessInfo>* processes);

private:
  std::vector<ProcessInfo>* processes_;

  std::vector<int> processPids_;
  Synchronizer* threadSync_;
  Core::SConfig config_;
  const std::string sensorConfigFile_;
  Exports::CExport exportObj_;
  Measurements::SAllSensors allSensors_;
  std::vector<Measurements::SMeasurementsData> measurementsData_;

  GStreamer::CPipelineMeasurements gstMeasurements_;
  Measurements::CSystemSensors sensorMeasurements_;
  Measurements::CProcessMeasurements processMeasurements_;
  std::vector<Core::SThreshold> thresholds_;

  Stopwatch testRunningTimer_;

  void Initialize();
  void StartMeasurementsLoop();
  void ExportData(const Exports::AllSensors& sensors,
                  const std::vector<Measurements::CCorrelation::SResult>& correlationResults);
  void AnalyzeData();
  void CheckTresholds();
  void OrganizeGstreamerPipelines();

  template <typename T>
  std::vector<T*> GetProcessFromProcesses() const
  {
    std::vector<T*> result;
    for (auto& e : *processes_)
    {
      if (auto process = std::get_if<T>(&e.processes))
        result.push_back(process);
    }
    return result;
  }

  void SetThresholdResults(const Measurements::SAllSensors& allSensors);
};

} // namespace Linux