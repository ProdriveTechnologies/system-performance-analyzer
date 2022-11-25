#include "process_measurements.h"

#include "linux/xavier_sensors.h"
#include "src/benchmarks/linux/performance_helpers.h"
#include "src/json_config/sensor_config/config_parser.h"

#include "src/benchmarks/linux/struct_sensors.h"

#include "src/linux/datahandlers/direct_handler.h"
#include "src/linux/datahandlers/pidstat_handler.h"
#include "src/linux/datahandlers/pidstatm_handler.h"

namespace Measurements
{
CProcessMeasurements::CProcessMeasurements(const std::string &configFile)
    : configFile_{configFile}
{
}

/**
 * @brief Initializes everything for measuring the processes
 *
 * @param allData
 */
void CProcessMeasurements::Initialize(
    std::vector<Exports::ExportData> *allData,
    std::vector<Linux::RunProcess *> processes)
{
  SetDataHandlers();
  processes_ = processes;
  allData_ = allData;
  auto parsed = PlatformConfig::Parse(configFile_);

  auto measureFields =
      GetFields(parsed.sensors, &CProcessMeasurements::GetMeasureFields, this);
  measureFields_ = measureFields.fields;
  measureFieldsDefinition_ = measureFields.definition;
  SetProcesses(); // Get the PIDs of the applications so they can be checked
  // Reset the proc-stat variables because they accumulate over-time
  // procHandler_.ParseProcStat();
  auto datahandlerMap = Linux::GetDatahandlerMap(dataHandlers_, "$PID$");
  dataHandler_.Initialize(datahandlerMap, measureFieldsDefinition_);
}

void CProcessMeasurements::SetDataHandlers()
{
  dataHandlers_.push_back(Linux::SDataHandlers{
      PlatformConfig::Types::DIRECT_PID,
      std::make_unique<Linux::CDirectHandler>(Linux::CDirectHandler())});
  dataHandlers_.push_back(Linux::SDataHandlers{
      PlatformConfig::Types::PID_STAT,
      std::make_unique<Linux::CPidStatHandler>(Linux::CPidStatHandler())});
  dataHandlers_.push_back(Linux::SDataHandlers{
      PlatformConfig::Types::PID_STATM,
      std::make_unique<Linux::CPidStatmHandler>(Linux::CPidStatmHandler())});
}

Exports::MeasurementItem CProcessMeasurements::GetConfig() const
{
  Exports::MeasurementItem config;
  config.name = "Process config";
  config.type = Exports::Type::INFO;
  config.value = GetMeasurementFields();
  return config;
}

std::vector<Exports::MeasurementItem>
CProcessMeasurements::GetMeasurementFields() const
{
  std::vector<Exports::MeasurementItem> result;
  for (const auto &process : processIds_)
  {
    Exports::MeasurementItem processInfo;
    processInfo.name = std::to_string(process.processId);
    processInfo.type = Exports::Type::ARRAY;
    std::vector<Exports::MeasurementItem> datapoints;
    for (const auto &e : measureFieldsDefinition_)
    {
      Exports::MeasurementItem config;
      config.name = e.name;
      config.type = Exports::Type::INFO;
      config.value = GetDefinitionItems(e, process.processId);
      datapoints.push_back(config);
    }
    processInfo.value = datapoints;
    result.push_back(processInfo);
  }

  return result;
}

std::vector<AllSensors::SensorGroups> CProcessMeasurements::GetSensors() const
{
  std::vector<AllSensors::SensorGroups> result;
  for (const auto &e : processIds_)
  {
    AllSensors::SensorGroups sensorGroup;
    sensorGroup.processId = e.userProcessId;

    for (const auto &datafield : measureFieldsDefinition_)
    {
      Sensors sensor{datafield};
      sensor.uniqueId = dataHandler_.GetUniqueId(e.processId, datafield.id);
      sensor.data = PerformanceHelpers::GetSummarizedDataProcesses(
          allData_, datafield.id);

      sensorGroup.sensors.push_back(sensor);
    }
    result.push_back(sensorGroup);
  }
  return result;
}

std::vector<Exports::MeasurementItem> CProcessMeasurements::GetDefinitionItems(
    const PlatformConfig::SDatafields &field, const int processId) const
{
  std::vector<Exports::MeasurementItem> result;
  auto item1 =
      Exports::MeasurementItem{"Label", Exports::Type::LABEL,
                               std::to_string(processId) + "." + field.name};
  result.push_back(item1);
  auto item2 =
      Exports::MeasurementItem{"Unique ID", Exports::Type::INFO, field.id};
  result.push_back(item2);
  if (!field.path.empty())
  {
    auto item3 =
        Exports::MeasurementItem{"Path", Exports::Type::INFO, field.path};
    result.push_back(item3);
  }
  return result;
}

void CProcessMeasurements::SetProcesses()
{
  for (const auto &process : processes_)
  {
    processIds_.push_back(ProcessDef{process->GetThreadPid(),
                                     process->GetProcessName(), true,
                                     process->GetUserProcessId()});
  }
}

/**
 * @brief Gets the measurements
 * 1. loops through each PID of each process and measures the content from the
 * SDatafields. It replaces the $PID$ tag with the pid of the processes
 * 2.
 *
 * @return std::vector<Exports::MeasuredItem>
 */
std::vector<Exports::ProcessInfo> CProcessMeasurements::GetMeasurements()
{
  std::vector<Exports::ProcessInfo> measuredItems;
  // 1. Loop through each PID
  for (auto &process : processIds_)
  {
    if (!process.active)
      continue;
    Exports::ProcessInfo processData;
    processData.pipelineId = process.processId;
    auto pidReplacement = std::to_string(process.processId);
    auto returnSuccess =
        dataHandler_.ParseMeasurements(pidReplacement, process.processId);
    if (returnSuccess)
    {
      processData.measuredItems = dataHandler_.GetMeasurements();
    }
    else
    {
      // Set process on inactive so it is not checked anymore, and continue with
      // next PID
      process.active = false;
      continue;
    }
    measuredItems.push_back(processData);
  }
  return measuredItems;
}

Linux::FileSystem::Stat CProcessMeasurements::GetProcStat(const int procId)
{
  const std::string procStatPath = "/proc/" + std::to_string(procId) + "/stat";
  auto stats = Linux::FileSystem::GetStats(procStatPath);
  return stats;
}
void CProcessMeasurements::SetInactive(const int processId)
{
  for (auto &e : processIds_)
  {
    if (e.processId == processId)
      e.active = false;
  }
}

CProcessMeasurements::MeasureCombo CProcessMeasurements::GetFields(
    std::vector<PlatformConfig::SDatafields> &sensorConfig,
    const std::function<MeasureCombo(CProcessMeasurements *,
                                     const PlatformConfig::SDatafields &)>
        parserFunction,
    CProcessMeasurements *memberPtr)
{
  MeasureCombo result;

  std::for_each(sensorConfig.begin(), sensorConfig.end(),
                [&](const PlatformConfig::SDatafields &dataField) {
                  result.Add(parserFunction(memberPtr, dataField));
                });

  return result;
}
CProcessMeasurements::MeasureCombo CProcessMeasurements::GetMeasureFields(
    const PlatformConfig::SDatafields &dataField)
{
  MeasureCombo result;

  switch (dataField.type)
  {
  case PlatformConfig::Types::ARRAY:
  {
    // If it doesn't contain any PID measurements (and is therefore empty), dont
    // add them at all
    auto datafields = ParseArray(dataField);
    if (!datafields.definition.empty())
      result.Add(datafields);
  }
  break;
  case PlatformConfig::Types::PID_STAT:
  case PlatformConfig::Types::DIRECT_PID:
  case PlatformConfig::Types::PID_STATM:
    result.Add(ParseField(dataField));
    break;
  case PlatformConfig::Types::PROC_STAT:
  case PlatformConfig::Types::PROC_MEM:
  case PlatformConfig::Types::DIRECT:
    // Do nothing, pass through to "default", these are system-wide measurements
  default:;
  }
  return result;
}

CProcessMeasurements::MeasureCombo
CProcessMeasurements::ParseArray(const PlatformConfig::SDatafields &data)
{
  MeasureCombo result;
  // Loop through the defined array
  for (size_t i = 0; i < data.size; ++i)
  {
    // Adjust all variables that are defined within the array
    for (const auto &e : data.datafields)
    {
      auto datafieldCopy{e};
      Helpers::replaceStr(datafieldCopy.path, "$INDEX$", std::to_string(i));
      datafieldCopy.nameClass = datafieldCopy.name;
      datafieldCopy.name = datafieldCopy.name + std::to_string(i);
      result.Add(GetMeasureFields(datafieldCopy));
    }
  }
  return result;
}

CProcessMeasurements::MeasureComboSingular
CProcessMeasurements::ParseField(const PlatformConfig::SDatafields &data)
{
  MeasureComboSingular result;
  result.field.path = data.path;
  result.field.type = data.type;
  result.field.id = PerformanceHelpers::GetUniqueId();
  result.definition = data;
  result.definition.id = result.field.id;
  return result;
}

} // namespace Measurements