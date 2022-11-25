#pragma once

#include <string>
#include <variant>

#include "export_struct.h"
#include "export_types.h"
#include "exports_base.h"
#include "file_writer.h"
#include "src/benchmarks/analysis/correlation.h"
#include "src/benchmarks/linux/struct_sensors.h"

#include "src/exports/export_types/export_csv.h"
#include "src/exports/export_types/export_graphs.h"
#include "src/exports/export_types/export_json.h"
#include "src/exports/export_types/summary_generator.h"

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
  CExport(CBase *exportObj, const std::string &fileName,
          const Core::SConfig &settings, const bool autoDeleteObj);
  CExport();
  ~CExport();

  void SetApplicationName(const std::string &name);

  // void Export(const ETypes exportType, const SExportData &exportData);

  void ExecuteExport(const Core::SExports &exportSettings,
                     const SExportData &exportData, const Core::SConfig &config)
  {
    if (!exportSettings.exportEnabled)
      return;
    if (exportSettings.exportType == ETypes::JSON)
      Export<CJson>(exportSettings, exportData, config);
    else if (exportSettings.exportType == ETypes::CSV)
      Export<CCsv>(exportSettings, exportData, config);
    else if (exportSettings.exportType == ETypes::TERMINAL_SUMMARY)
      Export<CSummaryGenerator>(exportSettings, exportData, config);
    else if (exportSettings.exportType == ETypes::GRAPHS)
      Export<CGraphs>(exportSettings, exportData, config);
  }

  template <typename ExportType>
  void Export(const Core::SExports &exportSettings,
              const SExportData &exportData, const Core::SConfig &config)
  {
    try
    {
      ExportType exportTypeClass{};
      exportTypeClass.SetApplicationName(exportSettings.filename);
      exportTypeClass.SetSettings(config);
      exportTypeClass.FullExport(exportData.config, exportData.data,
                                 exportData.allSensors,
                                 exportData.correlations);
    }
    catch (const std::exception &err)
    {
      CLogger::Log(CLogger::Types::ERROR, "Export failed: ", err.what());
    }
  }

private:
  FileWriter file_;
  const std::string fileName_;
  const bool autoDeleteObj_;

  CBase *pExportObj_;
};

inline CExport::CExport()
    : file_{""}, fileName_{""}, autoDeleteObj_{false}, pExportObj_{nullptr}
{
}

inline CExport::CExport(CBase *exportObj, const std::string &fileName,
                        const Core::SConfig &settings, const bool autoDeleteObj)
    : file_{fileName}, fileName_{fileName}, autoDeleteObj_{autoDeleteObj},
      pExportObj_{exportObj}
{
  pExportObj_->SetApplicationName(fileName_);
  pExportObj_->SetSettings(settings);
}
inline CExport::~CExport()
{
  if (autoDeleteObj_)
    delete pExportObj_;
}

inline void CExport::SetApplicationName(const std::string &name)
{
  pExportObj_->SetApplicationName(name);
}

} // namespace Exports