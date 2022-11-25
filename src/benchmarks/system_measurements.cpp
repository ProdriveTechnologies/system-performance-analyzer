#include "system_measurements.h"

#include "linux/xavier_sensors.h"
#include "src/benchmarks/linux/performance_helpers.h"
#include "src/json_config/sensor_config/config_parser.h"

#include "src/linux/datahandlers/direct_handler.h"
#include "src/linux/datahandlers/procmeminfo_handler.h"
#include "src/linux/datahandlers/procstat_handler.h"

namespace Measurements
{
CSensors::CSensors(const std::string &configFile) : configFile_{configFile} {}

void CSensors::Initialize(std::vector<Measurements::SMeasurementsData> *allData)
{
  SetDataHandlers();
  allData_ = allData;
  auto parsed = PlatformConfig::Parse(configFile_);
  auto measureFields =
      GetFields(parsed.sensors, &CSensors::GetMeasureFields, this);
  measureFields_ = measureFields.fields;
  measureFieldsDefinition_ = measureFields.definition;

  auto datahandlerMap = Linux::GetDatahandlerMap(dataHandlers_);
  dataHandler_.Initialize(datahandlerMap, measureFieldsDefinition_);
}

void CSensors::SetDataHandlers()
{
  dataHandlers_.push_back(Linux::SDataHandlers{
      PlatformConfig::ETypes::DIRECT,
      std::make_unique<Linux::CDirectHandler>(Linux::CDirectHandler())});
  dataHandlers_.push_back(
      Linux::SDataHandlers{PlatformConfig::ETypes::PROC_MEM,
                           std::make_unique<Linux::CProcMeminfoHandler>(
                               Linux::CProcMeminfoHandler())});
  dataHandlers_.push_back(Linux::SDataHandlers{
      PlatformConfig::ETypes::PROC_STAT,
      std::make_unique<Linux::CProcStatHandler>(Linux::CProcStatHandler())});
}

Exports::SMeasurementItem CSensors::GetConfig() const
{
  Exports::SMeasurementItem config;
  config.name = "SystemResources";
  config.type = Exports::EType::INFO;
  config.value = GetMeasurementFields();
  return config;
}

std::vector<Exports::SMeasurementItem> CSensors::GetMeasurementFields() const
{
  std::vector<Exports::SMeasurementItem> result;
  for (const auto &e : measureFieldsDefinition_)
  {
    Exports::SMeasurementItem config;
    config.name = e.name;
    config.type = Exports::EType::INFO;
    config.value = GetDefinitionItems(e);
    result.push_back(config);
  }
  return result;
}

std::vector<SSensors> CSensors::GetSensors(const bool summarizeData) const
{
  std::vector<SSensors> result;
  for (const auto &datafield : measureFieldsDefinition_)
  {
    SSensors sensor{datafield};
    if (summarizeData)
    {
      sensor.data = PerformanceHelpers::GetSummarizedData(
          Measurements::EClassification::SYSTEM, allData_, datafield.id,
          sensor.multiplier);
    }
    result.push_back(sensor);
  }
  // when not summarizing data, also don't create the collective groups
  if (!summarizeData)
    return result;
  // Add the collective groups, such as the combined cpu's instead of the single
  // cores
  std::unordered_map<std::string, std::unordered_set<int>> classes;
  for (const auto &e : measureFieldsDefinition_)
  {
    // Check if it is within an array, then these values are non-equal
    if (e.name != e.nameClass)
    {
      auto nameClass = classes.find(e.nameClass);
      if (nameClass != classes.end())
      {
        nameClass->second.insert(e.id);
      }
      else
      {
        classes.insert(
            std::make_pair(e.nameClass, std::unordered_set<int>{e.id}));
      }
    }
  }
  for (const auto &[className, classIds] : classes)
  {
    auto datafield = GetDatafield(className);
    datafield.name = className;
    result.push_back(PerformanceHelpers::GetSummarizedData(
        Measurements::EClassification::SYSTEM, allData_, classIds,
        Measurements::SSensors{datafield}));
  }
  return result;
}

std::vector<Exports::SMeasurementItem>
CSensors::GetDefinitionItems(const PlatformConfig::SDatafields &field) const
{
  std::vector<Exports::SMeasurementItem> result;
  auto item1 =
      Exports::SMeasurementItem{"Label", Exports::EType::LABEL, field.name};
  result.push_back(item1);
  auto item2 =
      Exports::SMeasurementItem{"Unique ID", Exports::EType::INFO, field.id};
  result.push_back(item2);
  if (!field.path.empty())
  {
    auto item3 =
        Exports::SMeasurementItem{"Path", Exports::EType::INFO, field.path};
    result.push_back(item3);
  }
  return result;
}

std::vector<SMeasuredItem> CSensors::GetMeasurements()
{
  std::vector<SMeasuredItem> items;
  auto returnSuccess = dataHandler_.ParseMeasurements();
  if (returnSuccess)
  {
    items = dataHandler_.GetMeasurements();
  }
  else
  {
    CLogger::Log(CLogger::Types::WARNING,
                 "Could not measure system measurements!");
  }
  return items;
}

CSensors::MeasureCombo CSensors::GetFields(
    std::vector<PlatformConfig::SDatafields> &sensorConfig,
    const std::function<MeasureCombo(CSensors *,
                                     const PlatformConfig::SDatafields &)>
        parserFunction,
    CSensors *memberPtr)
{
  MeasureCombo result;

  std::for_each(sensorConfig.begin(), sensorConfig.end(),
                [&](const PlatformConfig::SDatafields &dataField) {
                  result.Add(parserFunction(memberPtr, dataField));
                });

  return result;
}
CSensors::MeasureCombo
CSensors::GetMeasureFields(const PlatformConfig::SDatafields &dataField)
{
  MeasureCombo result;

  switch (dataField.type)
  {
  case PlatformConfig::ETypes::ARRAY:
  {
    // Check if it contains measurements for the system measurements (or if it's
    // empty or for specific processes)
    auto datafields = ParseArray(dataField);
    if (!datafields.definition.empty())
      result.Add(ParseArray(dataField));
  }
  break;
  case PlatformConfig::ETypes::DIRECT:
  case PlatformConfig::ETypes::PROC_STAT:
  case PlatformConfig::ETypes::PROC_MEM:
    result.Add(ParseField(dataField));
    break;
  default:;
  }
  return result;
}

CSensors::MeasureCombo
CSensors::ParseArray(const PlatformConfig::SDatafields &data)
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

CSensors::MeasureComboSingular
CSensors::ParseField(const PlatformConfig::SDatafields &data)
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