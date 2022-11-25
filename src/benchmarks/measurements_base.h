#pragma once

#include <vector>

#include "src/benchmarks/linux/struct_sensors.h"
#include "src/exports/export_struct.h"

namespace Measurements
{
class CClassificationBase
{
public:
  std::vector<Sensors> GetSensors() const;
  void Initialize(std::vector<Measurements::SMeasurementsData> *allData);

private:
  std::vector<Measurements::SMeasurementsData> *allData_;
};

} // namespace Measurements