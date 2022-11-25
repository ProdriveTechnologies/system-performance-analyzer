#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "src/benchmarks/linux/proc_handler.h"
#include "src/benchmarks/linux/struct_sensors.h"
#include "src/exports/export_struct.h"
#include "src/helpers/helper_functions.h"
#include "src/json_config/sensor_config/config.h"
#include "src/linux/data_handler.h"
#include "src/linux/run_process.h"

#include "src/linux/datahandlers/direct_handler.h"
#include "src/linux/datahandlers/pidstat_handler.h"
#include "src/linux/datahandlers/pidstatm_handler.h"

namespace Measurements
{
/**
 * @brief This class manages the measurements for the processes. It manages the
 * following tasks:
 * 1. Parse the JSON file for the sensor configuration, it only takes the
 * configurations related to specific processes
 * 2. Parse the measurements
 * 3. Check for the thresholds (not done in this class but the preparation is
 * done by this class)
 * 4. Support exports and analysis
 * 5. Detect performance decreases
 */
class CProcessMeasurements
{
public:
  //   CProcessMeasurements(std::vector<CGstreamerHandler *> gstreamerStream);
  CProcessMeasurements(const std::string &configFile);

  void Initialize(std::vector<Exports::ExportData> *allData,
                  std::vector<Linux::RunProcess *> processes);

  std::vector<Exports::PipelineInfo> ProcessMeasurements();
  Exports::MeasurementItem GetConfig() const;
  std::vector<Exports::ProcessInfo> GetMeasurements();

  std::vector<AllSensors::SensorGroups> GetSensors() const;

  // Should be removed
  std::vector<PlatformConfig::SDatafields> GetDefinition() const
  {
    return measureFieldsDefinition_;
  }

private:
  struct ProcessDef
  {
    int processId;
    std::string name;
    bool active = false;
  };
  std::vector<Linux::RunProcess *> processes_;
  std::string configFile_;
  std::vector<ProcessDef> processIds_;
  Measurements::ProcHandler procHandler_;
  Linux::CDataHandler dataHandler_;
  std::vector<Exports::ExportData> *allData_;

  using MeasureFieldsDefType = std::vector<PlatformConfig::SDatafields>;
  MeasureFieldsDefType measureFieldsDefinition_;
  using MeasureFieldsType = std::vector<PlatformConfig::SMeasureField>;
  MeasureFieldsType measureFields_;

  void SetProcesses();
  std::vector<Exports::MeasurementItem> GetMeasurementFields() const;
  std::vector<Exports::MeasurementItem>
  GetDefinitionItems(const PlatformConfig::SDatafields &field,
                     const int processId) const;
  void SetInactive(const int processId);

  std::vector<Linux::SDataHandlers> dataHandlers_;

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
  MeasureCombo GetFields(
      std::vector<PlatformConfig::SDatafields> &sensorConfig,
      const std::function<MeasureCombo(CProcessMeasurements *,
                                       const PlatformConfig::SDatafields &)>
          parserFunction,
      CProcessMeasurements *memberPtr);
  MeasureCombo GetMeasureFields(const PlatformConfig::SDatafields &dataField);
  MeasureCombo ParseArray(const PlatformConfig::SDatafields &data);
  MeasureComboSingular ParseField(const PlatformConfig::SDatafields &data);
  Linux::FileSystem::Stat GetProcStat(const int procId);
  void SetDataHandlers();
  std::unordered_map<PlatformConfig::Types, Linux::CDataHandler::Config>
  GetDatahandlerMap();
};
} // namespace Measurements
