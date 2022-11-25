#pragma once

#include <functional>
#include <string>
#include <vector>

class CXavierSensors
{
public:
  CXavierSensors(const int maxCores);
  struct CpuCoreInfo
  {
    double temperature;
    int frequency;
    int maxFrequency;
    int minFrequency;
    bool enabled;
  };
  enum class ECoreIdentifiers
  {
    ENABLED,
    MAX_FREQ,
    MIN_FREQ,
    CUR_FREQ,
    TEMPERATURE
  };
  enum class EIdentifiers
  {

  };
  struct Sensor
  {
    std::string name;
    EIdentifiers id;
    std::string path;
  };
  struct CoreSensor
  {
    std::string name;
    std::string prefixLocation;
    ECoreIdentifiers id;
    std::string path;
    std::function<void(CpuCoreInfo *, const std::string &)> storeResultFunction;
  };

  CpuCoreInfo GetCoreInfo(const unsigned int coreID);
  std::vector<CpuCoreInfo> GetCoresInfo();

private:
  std::vector<CoreSensor> coreSensors_;

  const int maxCores_;
  inline static const std::string preCoreNumberPath =
      "/sys/devices/system/cpu/cpu";
  inline static const std::string preCoreThermalPath =
      "/sys/class/thermal/thermal_zone";
  inline static const std::string coreEnabled = "";
};