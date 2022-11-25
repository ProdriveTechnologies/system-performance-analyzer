#pragma once

#include <string>
#include <vector>

#include "src/benchmarks/Linux/proc_handler.h"
#include "src/exports/export_struct.h"
#include "src/helpers/helper_functions.h"
#include "src/json_config/sensor_config/config.h"

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
class CSensors
{
public:
  //   CProcessMeasurements(std::vector<CGstreamerHandler *> gstreamerStream);
  CSensors(const std::string &configFile);

  void Initialize();

  std::vector<Exports::PipelineInfo> ProcessMeasurements();
  Exports::MeasurementItem GetConfig() const;
  std::vector<Exports::MeasuredItem> GetMeasurements();

  // Should be removed
  std::vector<PlatformConfig::SDatafields> GetDefinition() const
  {
    return measureFieldsDefinition_;
  }

private:
  const std::string configFile_;
  Measurements::ProcHandler procHandler_;

  using MeasureFieldsDefType = std::vector<PlatformConfig::SDatafields>;
  MeasureFieldsDefType measureFieldsDefinition_;
  using MeasureFieldsType = std::vector<PlatformConfig::SMeasureField>;
  MeasureFieldsType measureFields_;

  std::vector<Exports::MeasurementItem> GetMeasurementFields() const;
  std::vector<Exports::MeasurementItem>
  GetDefinitionItems(const PlatformConfig::SDatafields &field) const;
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

    void Add(const MeasureComboSingular &data)
    {
      definition.push_back(data.definition);
      fields.push_back(data.field);
    }
    void Add(const MeasureCombo &data)
    {
      definition = Helpers::CombineVectors(definition, data.definition);
      fields = Helpers::CombineVectors(fields, data.fields);
    }
  };
  PlatformConfig::SDatafields GetFieldDef(const int id)
  {
    for (const auto &e : measureFieldsDefinition_)
    {
      if (id == e.id)
        return e;
    }
    throw std::runtime_error("ID not found!");
  }
  MeasureCombo GetFields(std::vector<PlatformConfig::SDatafields> &sensorConfig,
                         const std::function<MeasureCombo(
                             CSensors *, const PlatformConfig::SDatafields &)>
                             parserFunction,
                         CSensors *memberPtr);
  MeasureCombo GetMeasureFields(const PlatformConfig::SDatafields &dataField);
  MeasureCombo ParseArray(const PlatformConfig::SDatafields &data);
  MeasureComboSingular ParseField(const PlatformConfig::SDatafields &data);
  std::vector<Exports::MeasuredItem>
  GetMeasurements(const MeasureFieldsType &measureFields);
};
} // namespace Measurements
