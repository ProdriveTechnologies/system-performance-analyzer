#pragma once

#include <string>
#include <variant>

#include "export_struct.h"
#include "exports_base.h"
#include "file_writer.h"
#include "src/benchmarks/analysis/correlation.h"
#include "src/benchmarks/linux/struct_sensors.h"

namespace Exports
{
class CExport
{
public:
  CExport(CBase *exportObj, const std::string &fileName,
          const Core::SConfig &settings, const bool autoDeleteObj);
  ~CExport();

  void SetApplicationName(const std::string &name);

  bool FullExport(
      const std::vector<MeasurementItem> &config, const FullMeasurement data,
      const AllSensors &allSensors,
      const std::vector<Measurements::CCorrelation::SResult> &correlations);

private:
  FileWriter file_;
  const std::string fileName_;
  const bool autoDeleteObj_;

  CBase *pExportObj_;
};

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

inline bool CExport::FullExport(
    const std::vector<MeasurementItem> &config, const FullMeasurement data,
    const AllSensors &allSensors,
    const std::vector<Measurements::CCorrelation::SResult> &correlations)
{
  return pExportObj_->FullExport(config, data, allSensors, correlations);
}

} // namespace Exports