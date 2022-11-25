#pragma once

#include <vector>

#include "src/benchmarks/Linux/struct_sensors.h"
#include "src/exports/export_struct.h"

namespace Measurements
{
class CClassificationBase
{
public:
  std::vector<Sensors> GetSensors() const;
  void Initialize(std::vector<Exports::ExportData> *allData);

private:
  std::vector<Exports::ExportData> *allData_;
};

} // namespace Measurements