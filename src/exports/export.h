#pragma once

#include <string>
#include <variant>

#include "export_struct.h"
#include "exports_base.h"
#include "file_writer.h"
#include "src/benchmarks/Linux/struct_sensors.h"

namespace Exports
{
class CExport
{
public:
  CExport(CBase *exportObj, const std::string &fileName,
          const bool autoDeleteObj);
  ~CExport();

  void SetApplicationName(const std::string &name);
  // bool InitExport(const std::vector<PlatformConfig::SDatafields> &config);
  // bool DataExport(const ExportData &data);
  // bool FinishExport();

  bool FullExport(const std::vector<MeasurementItem> &config,
                  const FullMeasurement data, const AllSensors &allSensors);

private:
  FileWriter file_;
  const std::string fileName_;
  const bool autoDeleteObj_;

  CBase *pExportObj_;
};

inline CExport::CExport(CBase *exportObj, const std::string &fileName,
                        const bool autoDeleteObj)
    : file_{fileName}, fileName_{fileName}, autoDeleteObj_{autoDeleteObj},
      pExportObj_{exportObj}
{
  pExportObj_->SetApplicationName(fileName_);
}
inline CExport::~CExport()
{
  if (autoDeleteObj_)
    delete pExportObj_;
}

// inline bool CExport::DataExport(const ExportData &data)
// {
//   return file_.AddRow(pExportObj_->ParseData(data), false);
// }
// inline bool
// CExport::InitExport(const std::vector<PlatformConfig::SDatafields> &config)
// {
//   return file_.AddRow(pExportObj_->InitExport(config), false);
// }
// inline bool CExport::FinishExport()
// {
//   return file_.AddRow(pExportObj_->FinishExport(), false);
// }
inline void CExport::SetApplicationName(const std::string &name)
{
  pExportObj_->SetApplicationName(name);
}

inline bool CExport::FullExport(const std::vector<MeasurementItem> &config,
                                const FullMeasurement data,
                                const AllSensors &allSensors)
{
  return pExportObj_->FullExport(config, data, allSensors);
}

} // namespace Exports