#include "xavier_sensors.h"
#include "Monitoring.h"

CXavierSensors::CXavierSensors()
    : coreSensors_{{"Enabled", ECoreIdentifiers::ENABLED, "/online",
                    [](CpuCoreInfo *coreInfo, const std::string &fileContent) {
                      coreInfo->enabled = fileContent == "1";
                    }}}
//  {"MinFreq", ECoreIdentifiers::MIN_FREQ, "/cpuinfo_min_freq"},
//  {"MaxFreq", ECoreIdentifiers::MIN_FREQ, "/cpuinfo_max_freq"},
//  {"CurrentFreq", ECoreIdentifiers::CUR_FREQ, "/cpuinfo_min_freq"}}
{
}

CXavierSensors::CpuCoreInfo
CXavierSensors::GetCoreInfo(const unsigned int coreID)
{
  std::string corePath = preCoreNumberPath + std::to_string(coreID);
  CpuCoreInfo result;

  for (const auto &e : coreSensors_)
  {
    std::string fileResult =
        Linux::CMonitoring::readLocation(corePath + e.path);
    e.storeResultFunction(&result, fileResult);
  }

  return result;
}