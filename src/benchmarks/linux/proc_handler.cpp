#include "proc_handler.h"

#include "performance_helpers.h"
#include "src/linux/filesystem.h"

namespace Measurements
{
void ProcHandler::ParseProcStat()
{
  auto currentProcStat = Linux::FileSystem::GetProcStat("/proc/stat");
  if (procStat_.cpus.size() != 0)
  {
    procStatCorrected_ = currentProcStat - procStat_;
  }
  procStat_ = currentProcStat;
}

/**
 * @brief Retrieves the meminfo structure from the /proc/meminfo file location
 *
 */
void ProcHandler::ParseMeminfo()
{
  meminfo_ = Linux::FileSystem::GetMemInfo("/proc/meminfo");
}
Exports::MeasuredItem
ProcHandler::ParseProcField(const PlatformConfig::SDatafields &procInfo,
                            const std::string &fieldName)
{
  auto cpuField = procStatCorrected_.cpus.find(fieldName);
  if (cpuField == procStatCorrected_.cpus.end())
  {
    throw std::runtime_error("Couldn't find field \"" + fieldName +
                             "\" for /proc/stat");
  }

  Exports::MeasuredItem measuredItem;
  measuredItem.id = procInfo.id;
  long long comparedTo = Linux::FileSystem::GetProcStatGroup(
      cpuField->second, procInfo.comparedTo);
  long long value =
      Linux::FileSystem::GetProcStatGroup(cpuField->second, procInfo.value);
  measuredItem.measuredValue = static_cast<double>(value) / comparedTo;
  return measuredItem;
}

Exports::MeasuredItem
ProcHandler::ParseMemField(const PlatformConfig::SDatafields &procInfo)
{
  auto memValue = meminfo_.GetField(procInfo.value);
  auto comparedTo = meminfo_.GetField(procInfo.comparedTo, true);

  Exports::MeasuredItem measuredItem;
  measuredItem.id = procInfo.id;
  measuredItem.measuredValue = static_cast<double>(memValue) / comparedTo;
  return measuredItem;
}

} // namespace Measurements