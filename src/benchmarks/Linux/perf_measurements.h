#pragma once

// #include "src/benchmarks/Monitoring.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "proc_handler.h"
#include "src/exports/export.h"
#include "src/exports/export_struct.h"
#include "src/helpers/stopwatch.h"
#include "src/helpers/timer.h"
#include "src/json_config/config.h"
#include "src/json_config/sensor_config/config.h"
// #include "xavier_sensors_live.h"
#include "src/benchmarks/gstreamer_measurements.h"
#include "src/benchmarks/sensor_measurements.h"
#include "src/processes_struct.h"

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

  CPerfMeasurements(Synchronizer *synchronizer,
                    const std::string &sensorConfig);
  void Start(const Core::SConfig &config, std::vector<ProcessInfo> *processes);

  // SCpuInfo GetCPUInfo();
  // SMemoryInfo GetMemoryInfo();
  // SBandwidth GetMemoryBandwidth();
  // SCoreTemperature GetTemperatures();

  struct ProcessMeasurements
  {
    int pid = -1;
    Linux::FileSystem::Stat stats;
    std::vector<int> threadIds;
  };
  struct ProcessesMeasure
  {
    std::vector<ProcessMeasurements> processes;
  };

private:
  std::vector<ProcessInfo> *processes_;
  std::vector<int> processPids_;
  Synchronizer *threadSync_;
  Core::SConfig config_;
  const std::string sensorConfigFile_;
  // CXavierSensors xavierSensors_;
  static constexpr int XAVIER_CORES = 8;
  Timer<> cpuUtilizationTimer_;

  // Measurements data
  std::unique_ptr<std::vector<Exports::ExportData>> pMeasurementsData_;

  // proc/stat measurements
  std::unique_ptr<std::vector<Linux::FileSystem::ProcStatData>> pCpuData_;

  // std::vector<std::string> excludedThreads_;
  std::unique_ptr<Exports::CExport> pExportObj_;
  // FileSystem::CLiveData<> liveFilesystemData_;

  using MeasureFieldsDefType = std::vector<PlatformConfig::SDatafields>;
  // MeasureFieldsDefType measureFieldsDefinition_;
  using MeasureFieldsType = std::vector<PlatformConfig::SMeasureField>;
  // MeasureFieldsType measureFields_;

  // MeasureFieldsDefType processFieldsDef_;
  // MeasureFieldsType processFields_;

  GStreamer::CProcessMeasurements gstMeasurements_;
  Exports::ExportConfig exportConfig_;
  Measurements::CSensors sensorMeasurements_;

  // PlatformConfig::SDatafields GetFieldDef(const int id)
  // {
  //   for (const auto &e : measureFieldsDefinition_)
  //   {
  //     if (id == e.id)
  //       return e;
  //   }
  //   throw std::runtime_error("ID not found!");
  // }

  // Measurements::ProcHandler procHandler_;
  Stopwatch testRunningTimer_;

  // struct MeasureComboSingular
  // {
  //   PlatformConfig::SDatafields definition;
  //   PlatformConfig::SMeasureField field;
  // };
  // struct MeasureCombo
  // {
  //   MeasureFieldsDefType definition;
  //   MeasureFieldsType fields;

  //   void Add(const MeasureComboSingular &data)
  //   {
  //     definition.push_back(data.definition);
  //     fields.push_back(data.field);
  //   }
  //   void Add(const MeasureCombo &data)
  //   {
  //     definition = Helpers::CombineVectors(definition, data.definition);
  //     fields = Helpers::CombineVectors(fields, data.fields);
  //   }
  // };

  std::unique_ptr<std::vector<ProcessesMeasure>> pProcessesData_;

  void Initialize();
  void StartMeasurementsLoop();
  void ExportData();
  void AnalyzeData();

  void CheckTresholds();

  void GetProcessPids();
  void RemoveProcessId(const int pid);

  void OrganizeGstreamerPipelines();

  void MeasureThread(const std::string &threadProcLoc);
  void MeasureProcesses(const std::vector<int> processIds);

  void InitExports(const MeasureFieldsDefType &config);
  void SendExportsData(const Exports::ExportData &data);
  // MeasureCombo GetFields(
  //     std::vector<PlatformConfig::SDatafields> &sensorConfig,
  //     const std::function<MeasureCombo(CPerfMeasurements *,
  //                                      const PlatformConfig::SDatafields &)>
  //         parserFunction,
  //     CPerfMeasurements *memberPtr);
  // MeasureCombo GetMeasureFields(const PlatformConfig::SDatafields &e);
  // MeasureCombo GetProcessFields(const PlatformConfig::SDatafields
  // &dataField);

  // MeasureCombo ParseArray(const PlatformConfig::SDatafields &data);
  // MeasureComboSingular ParseField(const PlatformConfig::SDatafields &data);
  // std::vector<Exports::MeasuredItem>
  // GetMeasuredItems(const MeasureFieldsType &measureFields);
};

} // namespace Linux