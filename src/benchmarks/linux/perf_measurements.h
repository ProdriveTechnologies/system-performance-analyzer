#pragma once

// #include "src/benchmarks/Monitoring.h"

#include "proc_handler.h"
#include "src/benchmarks/linux/struct_sensors.h"
#include "src/benchmarks/pipeline_measurements.h"
#include "src/benchmarks/process_measurements.h"
#include "src/benchmarks/system_measurements.h"
#include "src/exports/export.h"
#include "src/exports/export_struct.h"
#include "src/helpers/stopwatch.h"
#include "src/helpers/timer.h"
#include "src/json_config/config.h"
#include "src/json_config/sensor_config/config.h"
#include "src/processes_struct.h"
#include "struct_measurements.h"

#include <memory>
#include <string>
#include <unordered_map>
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

  // struct ProcessMeasurements
  // {
  //   int pid = -1;
  //   Linux::FileSystem::Stat stats;
  //   std::vector<int> threadIds;
  // };
  // struct ProcessesMeasure
  // {
  //   std::vector<ProcessMeasurements> processes;
  // };

private:
  std::vector<ProcessInfo>* processes_;

  std::vector<int> processPids_;
  Synchronizer* threadSync_;
  Core::SConfig config_;
  const std::string sensorConfigFile_;
  // CXavierSensors xavierSensors_;
  Timer<> cpuUtilizationTimer_;
  Exports::CExport exportObj_;
  Measurements::SAllSensors allSensors_;

  // Measurements data
  std::vector<Measurements::SMeasurementsData> measurementsData_;

  GStreamer::CPipelineMeasurements gstMeasurements_;
  Measurements::CSystemSensors sensorMeasurements_;
  Measurements::CProcessMeasurements processMeasurements_;
  std::vector<Core::SThreshold> thresholds_;

  Stopwatch testRunningTimer_;

  // std::unique_ptr<std::vector<ProcessesMeasure>> pProcessesData_;

  void Initialize();
  void StartMeasurementsLoop();
  void ExportData(const Exports::AllSensors& sensors,
                  const std::vector<Measurements::CCorrelation::SResult>& correlationResults);
  void AnalyzeData();
  void CheckTresholds();
  void RemoveProcessId(const int pid);
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

  void SetThresholdResults(Measurements::SAllSensors allSensors);
};

} // namespace Linux