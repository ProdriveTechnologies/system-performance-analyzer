#pragma once

// #include "src/benchmarks/Monitoring.h"

#include <memory>
#include <string>
#include <vector>

#include "src/exports/export.h"
#include "src/json_config/config.h"
#include "xavier_sensors.h"

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
  CXavierSensors xavierSensors_;
  static constexpr int XAVIER_CORES = 8;

  std::vector<std::string> excludedThreads_;
  std::unique_ptr<Exports::CExport> pExportObj_;

  void MeasureThread(const std::string &threadProcLoc);
  void MeasureSystem();

  void InitExports();
  void SendExportsData(const Exports::ExportData &data);

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