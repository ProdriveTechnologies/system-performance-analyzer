#pragma once

#include <vector>

#include "src/benchmarks/linux/struct_sensors.h"
#include "src/exports/export_struct.h"

namespace Measurements
{
class CClassificationBase
{
public:
  void SetDataPointer(std::vector<Measurements::SMeasurementsData> *allData)
  {
    allData_ = allData;
  }
  virtual std::vector<SSensors> GetSensors() const = 0;
  virtual std::vector<Measurements::SMeasurementGroup> GetMeasurements() = 0;

private:
  std::vector<Measurements::SMeasurementsData> *allData_;
};

} // namespace Measurements