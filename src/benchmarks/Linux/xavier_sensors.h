#pragma once

#include <functional>
#include <string>
#include <vector>

class CXavierSensors
{
public:
  CXavierSensors();
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
    ECoreIdentifiers id;
    std::string path;
    std::function<void(CpuCoreInfo *, const std::string &)> storeResultFunction;
  };

  CpuCoreInfo GetCoreInfo(const unsigned int coreID);
  std::vector<CpuCoreInfo> GetCoresInfo();

private:
  std::vector<CoreSensor> coreSensors_;

  inline static const std::string preCoreNumberPath =
      "/sys/devices/system/cpu/cpu";
  inline static const std::string coreEnabled = "";
};