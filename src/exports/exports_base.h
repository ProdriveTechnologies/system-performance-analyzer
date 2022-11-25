#pragma once

#include "export_struct.h"
#include "src/json_config/sensor_config/config.h"

namespace Exports
{
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

  virtual std::string InitExport(const PlatformConfig::SConfig &config) = 0;
  virtual std::string ParseData(const ExportData &data) = 0;
  virtual std::string FinishExport() = 0;
};

} // namespace Exports