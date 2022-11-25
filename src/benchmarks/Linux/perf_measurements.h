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
  void Start(const Core::SConfig &config);

  // SCpuInfo GetCPUInfo();
  // SMemoryInfo GetMemoryInfo();
  // SBandwidth GetMemoryBandwidth();
  // SCoreTemperature GetTemperatures();

  static std::string ReadLocation(const std::string &path);

private:
  Synchronizer *threadSync_;
  Core::SConfig config_;
  const std::string sensorConfigFile_;
  // CXavierSensors xavierSensors_;
  static constexpr int XAVIER_CORES = 8;
  Timer<> cpuUtilizationTimer_;
  // Linux::FileSystem::ProcStatData lastCpuDataAggregated_;
  // Linux::FileSystem::ProcStatData lastCpuData_;

  // Measurements data
  std::unique_ptr<std::vector<Exports::ExportData>> pMeasurementsData_;

  // proc/stat measurements
  std::unique_ptr<std::vector<Linux::FileSystem::ProcStatData>> pCpuData_;

  std::vector<std::string> excludedThreads_;
  std::unique_ptr<Exports::CExport> pExportObj_;
  // FileSystem::CLiveData<> liveFilesystemData_;

  using MeasureFieldsDefType = std::vector<PlatformConfig::SDatafields>;
  MeasureFieldsDefType measureFieldsDefinition_;
  using MeasureFieldsType = std::vector<PlatformConfig::SMeasureField>;
  MeasureFieldsType measureFields_;

  PlatformConfig::SDatafields GetFieldDef(const int id)
  {
    for (const auto &e : measureFieldsDefinition_)
    {
      if (id == e.id)
        return e;
    }
    throw std::runtime_error("ID not found!");
  }

  Measurements::ProcHandler procHandler_;
  Stopwatch testRunningTimer_;

  struct MeasureComboSingular
  {
    PlatformConfig::SDatafields definition;
    PlatformConfig::SMeasureField field;
  };
  struct MeasureCombo
  {
    MeasureFieldsDefType definition;
    MeasureFieldsType fields;

    void Add(const MeasureComboSingular &data)
    {
      definition.push_back(data.definition);
      fields.push_back(data.field);
    }
    void Add(const MeasureCombo &data)
    {
      definition = Helpers::CombineVectors(definition, data.definition);
      fields = Helpers::CombineVectors(fields, data.fields);
    }
  };

  void Initialize();
  void StartMeasurementsLoop();
  void ExportData();
  void AnalyzeData();

  void MeasureThread(const std::string &threadProcLoc);

  void InitExports(const MeasureFieldsDefType &config);
  void SendExportsData(const Exports::ExportData &data);
  MeasureCombo
  GetMeasureFields(const std::vector<PlatformConfig::SDatafields> &e);
  MeasureCombo GetMeasureFields(const PlatformConfig::SDatafields &e);

  MeasureCombo ParseArray(const PlatformConfig::SDatafields &data);
  // MeasureComboSingular ParseDirect(const PlatformConfig::SDatafields &data);
  // MeasureComboSingular ParseProcField(const PlatformConfig::SDatafields
  // &data);
  MeasureComboSingular ParseField(const PlatformConfig::SDatafields &data);
  std::vector<Exports::MeasuredItem>
  GetMeasuredItems(const MeasureFieldsType &measureFields);

  // static constexpr double tempNotAvailable_ = -1.0;
  // static constexpr double tempUnreadable = -2.0;
  // std::vector<SCoreTemperature> GetCpuTemperatures();
  // double GetCPUTemperature(const std::string &tempLocation);
  // static constexpr char tempLocation_[] =
  //     "/sys/class/thermal/thermal_zone*/temp";
  // static constexpr double tempToCelsiusDivider_ = 1000.0;
  // static constexpr int maxCpuCores_ = 64;
};

} // namespace Linux