#include "process_measurements.h"

#include "src/benchmarks/linux/performance_helpers.h"
#include "src/benchmarks/linux/struct_sensors.h"
#include "src/json_config/sensor_config/config_parser.h"
#include "src/linux/datahandlers/direct_handler.h"
#include "src/linux/datahandlers/pidstat_handler.h"
#include "src/linux/datahandlers/pidstatm_handler.h"

namespace Measurements
{
CProcessMeasurements::CProcessMeasurements(const std::string& configFile)
: configFile_{ configFile }
, allData_{ nullptr }
{
}

/**
 * @brief Initializes everything for measuring the processes
 *
 * @param allData
 */
void CProcessMeasurements::Initialize(std::vector<Measurements::SMeasurementsData>* allData,
                                      const std::vector<Linux::RunProcess*>& processes)
{
  SetDataHandlers();
  processes_ = processes;
  allData_ = allData;
  auto parsed = PlatformConfig::Parse(configFile_);

  auto measureFields = GetFields(parsed.sensors, &CProcessMeasurements::GetMeasureFields, this);
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
  dataHandlers_.push_back(Linux::SDataHandlers{ PlatformConfig::ETypes::DIRECT_PID,
                                                std::make_unique<Linux::CDirectHandler>(Linux::CDirectHandler()) });
  dataHandlers_.push_back(Linux::SDataHandlers{ PlatformConfig::ETypes::PID_STAT,
                                                std::make_unique<Linux::CPidStatHandler>(Linux::CPidStatHandler()) });
  dataHandlers_.push_back(Linux::SDataHandlers{ PlatformConfig::ETypes::PID_STATM,
                                                std::make_unique<Linux::CPidStatmHandler>(Linux::CPidStatmHandler()) });
}

Exports::SMeasurementItem CProcessMeasurements::GetConfig() const
{
  Exports::SMeasurementItem config;
  config.name = "ProcessMeasurements";
  config.type = Exports::EType::INFO;
  config.value = GetMeasurementFields();
  return config;
}

std::vector<Exports::SMeasurementItem> CProcessMeasurements::GetMeasurementFields() const
{
  std::vector<Exports::SMeasurementItem> result;
  for (const auto& process : processIds_)
  {
    Exports::SMeasurementItem processInfo;
    processInfo.name = std::to_string(process.processId);
    processInfo.type = Exports::EType::ARRAY;
    std::vector<Exports::SMeasurementItem> datapoints;
    for (const auto& e : measureFieldsDefinition_)
    {
      Exports::SMeasurementItem config;
      config.name = e.name;
      config.type = Exports::EType::INFO;
      config.value = GetDefinitionItems(e, process.processId);
      datapoints.push_back(config);
    }
    processInfo.value = datapoints;
    result.push_back(processInfo);
  }

  return result;
}

std::vector<SAllSensors::SSensorGroups> CProcessMeasurements::GetSensors(const bool summarizeData) const
{
  std::vector<SAllSensors::SSensorGroups> result;

  for (const auto& e : processIds_)
  {
    SAllSensors::SSensorGroups sensorGroup;
    sensorGroup.processId = e.userProcessId;
    sensorGroup.processDelay = GetProcessDelay(e.userProcessId);

    for (const auto& datafield : measureFieldsDefinition_)
    {
      try
      {
        SSensors sensor{ datafield };
        sensor.uniqueId = dataHandler_.GetUniqueId(e.processId, datafield.id);
        if (summarizeData)
        {
          sensor.data = PerformanceHelpers::GetSummarizedData(Measurements::EClassification::PROCESSES,
                                                              allData_,
                                                              sensor.uniqueId,
                                                              sensor.multiplier);
        }

        sensorGroup.sensors.push_back(sensor);
      }
      catch (const std::exception& error)
      {
        ; // Ignore the error and don't add the sensor
      }
    }
    result.push_back(sensorGroup);
  }
  return result;
}

/**
 * @brief Returns the delay of a process based on the ID
 *
 * @param processId
 * @return int
 */
int CProcessMeasurements::GetProcessDelay(const int processId) const
{
  for (const auto& e : processes_)
  {
    if (e->GetUserProcessId() == processId)
    {
      return e->GetProcessDelay();
    }
  }
  throw std::runtime_error("Could not find process with pipelineId");
}

std::vector<Exports::SMeasurementItem> CProcessMeasurements::GetDefinitionItems(
  const PlatformConfig::SDatafields& field,
  const int processId) const
{
  std::vector<Exports::SMeasurementItem> result;
  auto item1 =
    Exports::SMeasurementItem{ "Label", Exports::EType::LABEL, std::to_string(processId) + "." + field.name };
  result.push_back(item1);
  auto item2 = Exports::SMeasurementItem{ "Unique ID", Exports::EType::INFO, field.id };
  result.push_back(item2);
  if (!field.path.empty())
  {
    auto item3 = Exports::SMeasurementItem{ "Path", Exports::EType::INFO, field.path };
    result.push_back(item3);
  }
  return result;
}

void CProcessMeasurements::SetProcesses()
{
  for (const auto& process : processes_)
  {
    processIds_.push_back(
      ProcessDef{ process->GetApplicationPid(), process->GetProcessName(), true, process->GetUserProcessId() });
  }
}

/**
 * @brief Gets the measurements
 * 1. loops through each PID of each process and measures the content from the
 * SDatafields. It replaces the $PID$ tag with the pid of the processes
 * 2.
 *
 * @return std::vector<Measurements::SMeasuredItem>
 */
std::vector<Measurements::SMeasurementGroup> CProcessMeasurements::GetMeasurements()
{
  std::vector<Measurements::SMeasurementGroup> measuredItems;
  // 1. Loop through each PID
  for (auto& process : processIds_)
  {
    if (!process.active)
      continue;
    Measurements::SMeasurementGroup processData;
    processData.pipelineId = process.processId;
    auto pidReplacement = std::to_string(process.processId);
    auto returnSuccess = dataHandler_.ParseMeasurements(pidReplacement, process.processId);
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

CProcessMeasurements::MeasureCombo CProcessMeasurements::GetFields(
  std::vector<PlatformConfig::SDatafields>& sensorConfig,
  const std::function<MeasureCombo(CProcessMeasurements*, const PlatformConfig::SDatafields&)> parserFunction,
  CProcessMeasurements* memberPtr)
{
  MeasureCombo result;

  std::for_each(sensorConfig.begin(), sensorConfig.end(), [&](const PlatformConfig::SDatafields& dataField) {
    result.Add(parserFunction(memberPtr, dataField));
  });

  return result;
}
CProcessMeasurements::MeasureCombo CProcessMeasurements::GetMeasureFields(const PlatformConfig::SDatafields& dataField)
{
  MeasureCombo result;

  switch (dataField.type)
  {
  case PlatformConfig::ETypes::ARRAY:
  {
    // If it doesn't contain any PID measurements (and is therefore empty), dont
    // add them at all
    auto datafields = ParseArray(dataField);
    if (!datafields.definition.empty())
      result.Add(datafields);
  }
  break;
  case PlatformConfig::ETypes::PID_STAT:
  case PlatformConfig::ETypes::DIRECT_PID:
  case PlatformConfig::ETypes::PID_STATM:
    result.Add(ParseField(dataField));
    break;
  case PlatformConfig::ETypes::PROC_STAT:
  case PlatformConfig::ETypes::PROC_MEM:
  case PlatformConfig::ETypes::DIRECT:
    // Do nothing, pass through to "default", these are system-wide measurements
  default:;
  }
  return result;
}

CProcessMeasurements::MeasureCombo CProcessMeasurements::ParseArray(const PlatformConfig::SDatafields& data)
{
  MeasureCombo result;
  // Loop through the defined array
  for (size_t i = 0; i < data.size; ++i)
  {
    // Adjust all variables that are defined within the array
    for (const auto& e : data.datafields)
    {
      auto datafieldCopy{ e };
      Helpers::replaceStr(datafieldCopy.path, "$INDEX$", std::to_string(i));
      datafieldCopy.nameClass = datafieldCopy.name;
      datafieldCopy.name = datafieldCopy.name + std::to_string(i);
      result.Add(GetMeasureFields(datafieldCopy));
    }
  }
  return result;
}

CProcessMeasurements::MeasureComboSingular CProcessMeasurements::ParseField(const PlatformConfig::SDatafields& data)
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