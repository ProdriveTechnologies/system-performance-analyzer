#include "xavier_sensors.h"
#include "Monitoring.h"

CXavierSensors::CXavierSensors(const int maxCores)
    : coreSensors_{{"Enabled", preCoreNumberPath, ECoreIdentifiers::ENABLED,
                    "/online",
                    [](CpuCoreInfo *coreInfo, const std::string &fileContent) {
                      coreInfo->enabled = fileContent == "1";
                    }},
                   {"Current Frequency", preCoreNumberPath,
                    ECoreIdentifiers::CUR_FREQ, "/cpufreq/cpuinfo_cur_freq",
                    [](CpuCoreInfo *coreInfo, const std::string &fileContent) {
                      coreInfo->frequency = std::stoi(fileContent);
                    }},
                   {"Min Frequency", preCoreNumberPath,
                    ECoreIdentifiers::MIN_FREQ, "/cpufreq/cpuinfo_min_freq",
                    [](CpuCoreInfo *coreInfo, const std::string &fileContent) {
                      coreInfo->minFrequency = std::stoi(fileContent);
                    }},
                   {"Max Frequency", preCoreNumberPath,
                    ECoreIdentifiers::MAX_FREQ, "/cpufreq/cpuinfo_max_freq",
                    [](CpuCoreInfo *coreInfo, const std::string &fileContent) {
                      coreInfo->maxFrequency = std::stoi(fileContent);
                    }},
                   {"Temperature", preCoreThermalPath,
                    ECoreIdentifiers::TEMPERATURE, "/temp",
                    [](CpuCoreInfo *coreInfo, const std::string &fileContent) {
                      coreInfo->temperature = std::stoi(fileContent) / 1000.0;
                    }}},
      maxCores_{maxCores}
{
}

CXavierSensors::CpuCoreInfo
CXavierSensors::GetCoreInfo(const unsigned int coreID)
{
  const std::string coreNr = std::to_string(coreID);
  CpuCoreInfo result;

  for (const auto &e : coreSensors_)
  {
    const std::string &fullPath{e.prefixLocation + coreNr + e.path};
    const std::string fileResult =
        Linux::CMonitoring::readLocation(e.prefixLocation + coreNr + e.path);
    e.storeResultFunction(&result, fileResult);
  }

  return result;
}

std::vector<CXavierSensors::CpuCoreInfo> CXavierSensors::GetCoresInfo()
{
  std::vector<CpuCoreInfo> coresInfo;
  for (int i = 0; i < maxCores_; ++i)
  {
    coresInfo.push_back(GetCoreInfo(i));
  }
  return coresInfo;
}