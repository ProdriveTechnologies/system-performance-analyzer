#include "proc_handler.h"

#include "src/linux/filesystem.h"
namespace Measurements
{
void ProcHandler::ParseProcStat()
{
  procStat_ = Linux::FileSystem::GetProcStat();
}

std::vector<Exports::MeasuredItem>
ProcHandler::ParseProcField(const PlatformConfig::SDatafields &procInfo)
{
  if (procInfo.size == 0)
    return {ParseProcField(procInfo, procInfo.field)};
  else
  {
    std::vector<Exports::MeasuredItem> items;
    for (size_t i = 0; i < procInfo.size; i++)
    {
      std::string fieldName = procInfo.field;
      Helpers::replaceStr(fieldName, "$INDEX$", std::to_string(i));
      items.push_back(ParseProcField(procInfo, fieldName));
    }
    return items;
  }
}

Exports::MeasuredItem
ProcHandler::ParseProcField(const PlatformConfig::SDatafields &procInfo,
                            const std::string &fieldName)
{
  auto cpuField = procStat_.cpus.find(fieldName);
  if (cpuField == procStat_.cpus.end())
  {
    throw std::runtime_error("Couldn't find field \"" + fieldName +
                             "\" for /proc/stat");
  }

  Exports::MeasuredItem measuredItem;
  measuredItem.id = 340;
  measuredItem.measuredValue = cpuField->second.jiffiesIdle;
  return measuredItem;
}

} // namespace Measurements