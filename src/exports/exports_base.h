#pragma once

#include <vector>

#include "export_struct.h"
#include "src/benchmarks/linux/struct_sensors.h"
#include "src/json_config/sensor_config/config.h"

namespace Exports
{
using FullMeasurement = std::vector<Exports::ExportData> *;
using AllSensors = Measurements::AllSensors;
/**
 * @brief CBase the base class for the exports,
 * contains the data structure for the exports data
 *
 * @tparam ExportType the type of export that will be used. Can be any export
 * class (CSV, JSON)
 */
class CBase
{
public:
  CBase() {}
  virtual ~CBase() {}

  void SetApplicationName(const std::string &filename) { filename_ = filename; }
  // virtual std::string
  // InitExport(const std::vector<PlatformConfig::SDatafields> &config) = 0;
  // virtual std::string ParseData(const ExportData &data) = 0;
  // virtual std::string FinishExport() = 0;
  virtual bool FullExport(const std::vector<MeasurementItem> &config,
                          const FullMeasurement data,
                          const Measurements::AllSensors &allSensors) = 0;

protected:
  std::string filename_;
};

} // namespace Exports