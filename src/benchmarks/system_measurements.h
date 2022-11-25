#pragma once

#include "src/benchmarks/linux/struct_measurements.h"
#include "src/benchmarks/linux/struct_sensors.h"
#include "src/exports/export_struct.h"
#include "src/helpers/helper_functions.h"
#include "src/json_config/sensor_config/config.h"
#include "src/linux/data_handler.h"
#include "src/linux/datahandlers/direct_handler.h"
#include "src/linux/datahandlers/pidstat_handler.h"
#include "src/linux/datahandlers/pidstatm_handler.h"

#include <string>
#include <vector>

namespace Measurements
{
/**
 * @brief This class manages the measurements from the sensors. It manages the
 * following tasks:
 * 1. Parse the JSON file for the sensor configuration
 * 2. Parse the measurements
 * 3. Check for the thresholds
 * 4. Support exports and analysis
 * 5. Detect performance decreases
 */
class CSystemSensors
{
public:
  explicit CSystemSensors(const std::string& configFile);

  void Initialize(std::vector<Measurements::SMeasurementsData>* allData);

  Exports::SMeasurementItem GetConfig() const;

  std::vector<Measurements::SMeasuredItem> GetMeasurements();

  std::vector<SSensors> GetSensors(const bool summarizeData = true) const;

private:
  const std::string configFile_;
  std::vector<Measurements::SMeasurementsData>* allData_;
  Linux::CDataHandler dataHandler_;
  std::vector<Linux::SDataHandlers> dataHandlers_;

  using MeasureFieldsDefType = std::vector<PlatformConfig::SDatafields>;
  MeasureFieldsDefType measureFieldsDefinition_;
  using MeasureFieldsType = std::vector<PlatformConfig::SMeasureField>;
  MeasureFieldsType measureFields_;

  std::vector<Exports::SMeasurementItem> GetMeasurementFields() const;
  std::vector<Exports::SMeasurementItem> GetDefinitionItems(const PlatformConfig::SDatafields& field) const;
  /**
   * @brief Necessary for parsing the configuration file for the sensors
   */
  struct MeasureComboSingular
  {
    PlatformConfig::SDatafields definition;
    PlatformConfig::SMeasureField field;
  };
  struct MeasureCombo
  {
    MeasureFieldsDefType definition;
    MeasureFieldsType fields;

    void Add(const MeasureComboSingular& data)
    {
      definition.push_back(data.definition);
      fields.push_back(data.field);
    }
    void Add(const MeasureCombo& data)
    {
      definition = Helpers::CombineVectors(definition, data.definition);
      fields = Helpers::CombineVectors(fields, data.fields);
    }
  };
  PlatformConfig::SDatafields GetFieldDef(const int id)
  {
    for (const auto& e : measureFieldsDefinition_)
    {
      if (id == e.id)
        return e;
    }
    throw std::runtime_error("ID not found!");
  }
  MeasureCombo GetFields(
    std::vector<PlatformConfig::SDatafields>& sensorConfig,
    const std::function<MeasureCombo(CSystemSensors*, const PlatformConfig::SDatafields&)> parserFunction,
    CSystemSensors* memberPtr);
  MeasureCombo GetMeasureFields(const PlatformConfig::SDatafields& dataField);
  MeasureCombo ParseArray(const PlatformConfig::SDatafields& data);
  MeasureComboSingular ParseField(const PlatformConfig::SDatafields& data);
  void SetDataHandlers();
  PlatformConfig::SDatafields GetDatafield(const std::string& className) const
  {
    for (const auto& datafield : measureFieldsDefinition_)
    {
      if (datafield.nameClass == className)
      {
        return datafield;
      }
    }
    throw std::runtime_error("Could not find datafield!");
  }
};
} // namespace Measurements
