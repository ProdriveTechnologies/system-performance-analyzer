#include "system_measurements.h"

#include "Linux/xavier_sensors.h"
#include "src/benchmarks/Linux/performance_helpers.h"
#include "src/json_config/sensor_config/config_parser.h"

namespace Measurements
{
CSensors::CSensors(const std::string &configFile) : configFile_{configFile} {}

void CSensors::Initialize(std::vector<Exports::ExportData> *allData)
{
  allData_ = allData;
  auto parsed = PlatformConfig::Parse(configFile_);
  auto measureFields =
      GetFields(parsed.sensors, &CSensors::GetMeasureFields, this);
  measureFields_ = measureFields.fields;
  measureFieldsDefinition_ = measureFields.definition;
  // Reset the proc-stat variables because they accumulate over-time
  procHandler_.ParseProcStat();
}

Exports::MeasurementItem CSensors::GetConfig() const
{
  Exports::MeasurementItem config;
  config.name = "Measurement fields";
  config.type = Exports::Type::INFO;
  config.value = GetMeasurementFields();
  return config;
}

std::vector<Exports::MeasurementItem> CSensors::GetMeasurementFields() const
{
  std::vector<Exports::MeasurementItem> result;
  for (const auto &e : measureFieldsDefinition_)
  {
    Exports::MeasurementItem config;
    config.name = e.name;
    config.type = Exports::Type::INFO;
    config.value = GetDefinitionItems(e);
    result.push_back(config);
  }
  return result;
}

std::vector<Sensors> CSensors::GetSensors() const
{
  std::vector<Sensors> result;
  for (const auto &datafield : measureFieldsDefinition_)
  {
    Sensors sensor{datafield};
    sensor.data =
        PerformanceHelpers::GetSummarizedDataSensors(allData_, datafield.id);
    result.push_back(sensor);
  }
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
    result.push_back(PerformanceHelpers::GetSummarizedDataSensors(
        allData_, classIds, className, datafield));
  }
  return result;
}

std::vector<Exports::MeasurementItem>
CSensors::GetDefinitionItems(const PlatformConfig::SDatafields &field) const
{
  std::vector<Exports::MeasurementItem> result;
  auto item1 =
      Exports::MeasurementItem{"Label", Exports::Type::LABEL, field.name};
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

std::vector<Exports::MeasuredItem> CSensors::GetMeasurements()
{
  procHandler_.ParseMeminfo();
  return GetMeasurements(measureFields_);
}

std::vector<Exports::MeasuredItem>
CSensors::GetMeasurements(const MeasureFieldsType &measureFields)
{
  procHandler_.ParseProcStat();
  std::vector<Exports::MeasuredItem> measuredItems;
  for (const auto &e : measureFields)
  {
    switch (e.type)
    {
    case PlatformConfig::Types::DIRECT:
      measuredItems.push_back(
          CXavierSensors::ParseDirect(e)); // ParseDirect(e);
      break;
    case PlatformConfig::Types::PROC_STAT:
    {
      auto definition = GetFieldDef(e.id);
      measuredItems.push_back(procHandler_.ParseProcField(definition, e.path));
    }
    break;
    case PlatformConfig::Types::PROC_MEM:
    {
      auto definition = GetFieldDef(e.id);
      measuredItems.push_back(procHandler_.ParseMemField(definition));
    }
    break;
    default:
      throw std::runtime_error(
          "Software Error! Incorrect type in the performance measurements!");
    }
  }
  return measuredItems;
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
  case PlatformConfig::Types::ARRAY:
  {
    // Check if it contains measurements for the system measurements (or if it's
    // empty or for specific processes)
    auto datafields = ParseArray(dataField);
    if (!datafields.definition.empty())
      result.Add(ParseArray(dataField));
  }
  break;
  case PlatformConfig::Types::DIRECT:
  case PlatformConfig::Types::PROC_STAT:
  case PlatformConfig::Types::PROC_MEM:
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