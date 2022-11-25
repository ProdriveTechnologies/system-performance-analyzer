#pragma once

#include <string>
#include <variant>

#include "export_struct.h"
#include "exports_base.h"
#include "file_writer.h"

namespace Exports
{
class CExport
{
public:
  CExport(CBase *exportObj, const std::string &fileName,
          const bool autoDeleteObj);
  ~CExport();

  bool InitExport(const PlatformConfig::SConfig &config);
  bool DataExport(const ExportData &data);
  bool FinishExport();

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
}
inline CExport::~CExport()
{
  if (autoDeleteObj_)
    delete pExportObj_;
}

inline bool CExport::DataExport(const ExportData &data)
{
  return file_.AddRow(pExportObj_->ParseData(data), false);
}
inline bool CExport::InitExport(const PlatformConfig::SConfig &config)
{
  return file_.AddRow(pExportObj_->InitExport(config), false);
}
inline bool CExport::FinishExport()
{
  return file_.AddRow(pExportObj_->FinishExport(), false);
}

} // namespace Exports