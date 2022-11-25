#include "Monitoring.h"

#include "Helpers.h"
#include <regex> // std::regex_replace
namespace Linux
{
CMonitoring::SCpuInfo CMonitoring::GetCPUInfo()
{
  SCpuInfo cpuInfo;

  std::string location = "/proc/" + _applicationPID + "/stat";
  std::fstream file(location, std::fstream::in);
  if (file.good())
  {
    std::string cpuProcess;
    getline(file, cpuProcess);
    std::vector<int> vectored =
        stringToVector(deleteNonNumericAndSpace(cpuProcess));
    if (vectored.size() < 20)
    {
      return {};
    }
    int procTimes = vectored.at(12) + vectored.at(13);
    return procTimes;
  }
  throw std::runtime_error("Couldn't open file!");
}
CMonitoring::SMemoryInfo CMonitoring::GetMemoryInfo() { return {}; }
CMonitoring::SBandwidth CMonitoring::GetMemoryBandwidth() { return {}; }

/**
 * \brief temperature returns the temperature of the processor
 * \return a double with the processor temperature in degrees Celsius or -1 when
 * an error occurs
 */
double CMonitoring::GetCPUTemperature(const std::string &tempLocation)
{
  std::string buffer;
  std::ifstream temp(tempLocation);
  if (!temp.is_open() || !temp.good())
  {
    return tempNotAvailable_;
  }
  temp >> buffer;
  temp.close();

  if (buffer.empty())
    return tempUnreadable_;
  return (std::stoi(buffer) / tempToCelsiusDivider_);
}

std::vector<CMonitoring::SCoreTemperature> CMonitoring::GetCpuTemperatures()
{
  std::vector<SCoreTemperature> temperatures;
  for (int i = 0; i < maxCpUCores_; i++)
  {
    std::string temperaturePath =
        std::regex_replace(tempLocation_, std::regex("*"), std::to_string(i));
    auto cpuTemperature = GetCPUTemperature();
    if (cpuTemperature < 0)
      break;
    temperatures.push_back(SCoreTemperature(i, cpuTemperature));
  }
  return temperatures;
}

} // namespace Linux