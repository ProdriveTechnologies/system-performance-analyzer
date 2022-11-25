#include "proc_handler.h"

#include "src/linux/filesystem.h"
namespace Measurements
{
void ProcHandler::ParseProcStat()
{
  auto currentProcStat = Linux::FileSystem::GetProcStat();
  if (procStat_.cpus.size() != 0)
  {
    procStatCorrected_ = currentProcStat - procStat_;
  }
  procStat_ = currentProcStat;
}

std::vector<PlatformConfig::SDatafields>
ProcHandler::ParseProcField(const PlatformConfig::SDatafields &procInfo)
{
  if (procInfo.size == 0)
    return {procInfo};
  else
  {
    std::vector<PlatformConfig::SDatafields> items;
    for (size_t i = 0; i < procInfo.size; i++)
    {
      PlatformConfig::SDatafields dataFields{procInfo};
      Helpers::replaceStr(dataFields.path, "$INDEX$", std::to_string(i));
      items.push_back(dataFields);
    }
    return items;
  }
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
  measuredItem.id = 340;
  measuredItem.measuredValue = cpuField->second.jiffiesIdle;
  long long comparedTo = Linux::FileSystem::GetProcStatGroup(
      cpuField->second, procInfo.comparedTo);
  long long value =
      Linux::FileSystem::GetProcStatGroup(cpuField->second, procInfo.value);
  measuredItem.measuredValue = static_cast<double>(value) / comparedTo;
  return measuredItem;
}

} // namespace Measurements