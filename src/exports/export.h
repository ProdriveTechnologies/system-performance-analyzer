#pragma once

#include <string>
#include <variant>

#include "export_struct.h"
#include "export_types.h"
#include "exports_base.h"
#include "src/benchmarks/analysis/correlation.h"
#include "src/benchmarks/linux/struct_sensors.h"

#include "src/exports/export_types/export_csv.h"
#include "src/exports/export_types/export_graphs.h"
#include "src/exports/export_types/export_json.h"
#include "src/exports/export_types/summary_generator.h"
#include "src/exports/export_types/terminal_ui.h"

namespace Exports
{
struct SExportData
{
  const std::vector<MeasurementItem> &config;
  const FullMeasurement data;
  const AllSensors &allSensors;
  const std::vector<Measurements::CCorrelation::SResult> &correlations;
};

class CExport
{
public:
  CExport();
  ~CExport();

  void InitialiseLiveMeasurements(Measurements::AllSensors *sensors,
                                  const Core::SConfig &config);

  void AddMeasurements([
      [maybe_unused]] const Measurements::SMeasurementsData data)
  {
    for (auto &obj : liveTypes_)
    {
      obj->AddMeasurements(data);
    }
  }
  void FinishLiveMeasurements()
  {
    for (auto &obj : liveTypes_)
    {
      obj->FinishLiveMeasurements();
    }
  }
  // void Export(const ETypes exportType, const SExportData &exportData);

  void ExecuteExport(const Core::SExports &exportSettings,
                     const SExportData &exportData, const Core::SConfig &config)
  {
    if (!exportSettings.exportEnabled)
      return;
    auto res = exportObjects_.find(exportSettings.exportType);
    if (res != exportObjects_.end())
    {
      // The visit construction is necessary because the object is contained
      // within a std::variant
      std::visit(
          Overload{
              [&](auto &exportObj) {
                exportObj.SetApplicationName(exportSettings.filename);
                exportObj.SetSettings(config);
                exportObj.FullExport(exportData.config, exportData.data,
                                     exportData.allSensors,
                                     exportData.correlations);
              },
          },
          res->second);
    }
  }

private:
  using ExportObjs =
      std::variant<CJson, CCsv, CSummaryGenerator, CGraphs, CTerminalUI>;
  std::unordered_map<ETypes, ExportObjs> exportObjects_;

  std::vector<CBase *> liveTypes_;

  void CreateObjects();
  void SetLiveModeObjects();
};

inline CExport::CExport()
{
  // Add all export objects and their types
  exportObjects_.insert({ETypes::JSON, CJson{}});
  exportObjects_.insert({ETypes::CSV, CCsv{}});
  exportObjects_.insert({ETypes::TERMINAL_SUMMARY, CSummaryGenerator{}});
  exportObjects_.insert({ETypes::GRAPHS, CGraphs{}});
  exportObjects_.insert({ETypes::TERMINAL_UI, CTerminalUI{}});
}

inline CExport::~CExport() {}

/**
 * @brief
 *
 */
inline void
CExport::InitialiseLiveMeasurements(Measurements::AllSensors *sensors,
                                    const Core::SConfig &settings)
{
  SetLiveModeObjects();
  std::cout << "Initialize live measurements" << std::endl;
  for (auto &obj : liveTypes_)
  {
    obj->SetSettings(settings);
    obj->SetSensorConfig(sensors);
    std::cout << "Start live measurements" << std::endl;
    obj->StartLiveMeasurements();
  }
}

/**
 * @brief fills the liveTypes_ vector with all the objects where GetLiveMode() =
 * true
 *
 */
inline void CExport::SetLiveModeObjects()
{
  for (auto &e : exportObjects_)
  {
    std::visit(
        Overload{
            [this](auto &exportObj) {
              if (exportObj.GetLiveMode())
              {
                liveTypes_.push_back(&exportObj);
              }
            },
        },
        e.second);
  }
}

} // namespace Exports