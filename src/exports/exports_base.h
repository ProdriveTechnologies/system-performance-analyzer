#pragma once

#include <vector>

#include "export_struct.h"
#include "src/benchmarks/linux/struct_sensors.h"
#include "src/json_config/config.h"
#include "src/json_config/sensor_config/config.h"

#include "src/benchmarks/analysis/correlation.h"

namespace Exports
{
using FullMeasurement = std::vector<Measurements::SMeasurementsData> *;
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
  CBase() : hasLiveMode_{false} {}
  virtual ~CBase() {}

  void SetApplicationName(const std::string &filename) { filename_ = filename; }
  void SetSettings(const Core::SConfig &settings) { settings_ = settings; }
  bool GetLiveMode() const { return hasLiveMode_; }

  void SetSensorConfig(Measurements::AllSensors *sensors)
  {
    liveSensors_ = sensors;
  }

  virtual void StartLiveMeasurements() {}
  virtual void AddMeasurements([
      [maybe_unused]] const Measurements::SMeasurementsData data)
  {
  }
  virtual void FinishLiveMeasurements() {}

  virtual bool FullExport(
      const std::vector<MeasurementItem> &config, const FullMeasurement data,
      const Measurements::AllSensors &allSensors,
      const std::vector<Measurements::CCorrelation::SResult> &correlations) = 0;

protected:
  std::string filename_;
  Core::SConfig settings_;
  bool hasLiveMode_;
  Measurements::AllSensors *liveSensors_;

  void enableLiveMode() { hasLiveMode_ = true; }
};

} // namespace Exports