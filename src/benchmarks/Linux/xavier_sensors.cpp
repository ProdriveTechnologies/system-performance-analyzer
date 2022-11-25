#include "xavier_sensors.h"
#include "perf_measurements.h"

// CXavierSensors::CXavierSensors(const int maxCores)
//     : coreSensors_{{"Enabled", preCoreNumberPath, ECoreIdentifiers::ENABLED,
//                     "/online",
//                     [](SCpuCoreInfo *coreInfo, const std::string
//                     &fileContent) {
//                       coreInfo->enabled = fileContent == "1";
//                     }},
//                    {"Current Frequency", preCoreNumberPath,
//                     ECoreIdentifiers::CUR_FREQ, "/cpufreq/cpuinfo_cur_freq",
//                     [](SCpuCoreInfo *coreInfo, const std::string
//                     &fileContent) {
//                       coreInfo->frequency = std::stoi(fileContent);
//                     }},
//                    {"Min Frequency", preCoreNumberPath,
//                     ECoreIdentifiers::MIN_FREQ, "/cpufreq/cpuinfo_min_freq",
//                     [](SCpuCoreInfo *coreInfo, const std::string
//                     &fileContent) {
//                       coreInfo->minFrequency = std::stoi(fileContent);
//                     }},
//                    {"Max Frequency", preCoreNumberPath,
//                     ECoreIdentifiers::MAX_FREQ, "/cpufreq/cpuinfo_max_freq",
//                     [](SCpuCoreInfo *coreInfo, const std::string
//                     &fileContent) {
//                       coreInfo->maxFrequency = std::stoi(fileContent);
//                     }},
//                    {"Temperature", preCoreThermalPath,
//                     ECoreIdentifiers::TEMPERATURE, "/temp",
//                     [](SCpuCoreInfo *coreInfo, const std::string
//                     &fileContent) {
//                       coreInfo->temperature = std::stoi(fileContent) /
//                       1000.0;
//                     }}},
//       maxCores_{maxCores}
// {
// }

// CXavierSensors::SCpuCoreInfo
// CXavierSensors::GetCoreInfo(const unsigned int coreID)
// {
//   const std::string coreNr = std::to_string(coreID);
//   SCpuCoreInfo result;

//   for (const auto &e : coreSensors_)
//   {
//     const std::string fileResult = Linux::CPerfMeasurements::ReadLocation(
//         e.prefixLocation + coreNr + e.path);
//     e.storeResultFunction(&result, fileResult);
//   }

//   return result;
// }

// std::vector<CXavierSensors::SCpuCoreInfo> CXavierSensors::GetCoresInfo()
// {
//   std::vector<SCpuCoreInfo> coresInfo;
//   for (int i = 0; i < maxCores_; ++i)
//   {
//     coresInfo.push_back(GetCoreInfo(i));
//   }
//   return coresInfo;
// }

Exports::MeasuredItem
CXavierSensors::ParseDirect(const PlatformConfig::SMeasureField &datafield)
{
  Exports::MeasuredItem item;
  item.id = datafield.id;
  item.measuredValue =
      std::stoi(Linux::CPerfMeasurements::ReadLocation(datafield.path));
  return item;
}