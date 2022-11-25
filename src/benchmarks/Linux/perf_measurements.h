#pragma once

// #include "src/benchmarks/Monitoring.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/exports/export.h"
#include "src/exports/export_struct.h"
#include "src/helpers/timer.h"
#include "src/json_config/config.h"
#include "src/json_config/sensor_config/config.h"
#include "xavier_sensors_live.h"

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

  CPerfMeasurements(Synchronizer *synchronizer);
  void Start(const Core::SConfig &config);

  // SCpuInfo GetCPUInfo();
  // SMemoryInfo GetMemoryInfo();
  // SBandwidth GetMemoryBandwidth();
  // SCoreTemperature GetTemperatures();

  static std::string ReadLocation(const std::string &path);

private:
  Synchronizer *threadSync_;
  Core::SConfig config_;
  // CXavierSensors xavierSensors_;
  static constexpr int XAVIER_CORES = 8;
  Timer<> cpuUtilizationTimer_;
  Linux::FileSystem::ProcStatData lastCpuDataAggregated_;
  Linux::FileSystem::ProcStatData lastCpuData_;

  std::vector<std::string> excludedThreads_;
  std::unique_ptr<Exports::CExport> pExportObj_;
  FileSystem::CLiveData<> liveFilesystemData_;

  using MeasureFieldsType = std::vector<PlatformConfig::SDatafields>;
  MeasureFieldsType measureFields_;

  void MeasureThread(const std::string &threadProcLoc);
  void MeasureSystem();

  void InitExports(const PlatformConfig::SConfig &config);
  void SendExportsData(const Exports::ExportData &data);
  std::vector<PlatformConfig::SDatafields>
  GetMeasureFields(const PlatformConfig::SConfig &e);

  std::vector<PlatformConfig::SDatafields>
  ParseArray(const PlatformConfig::SDatafields &data);
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