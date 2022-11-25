#pragma once

#include "src/benchmarks/linux/struct_measurements.h"
#include "src/benchmarks/linux/struct_sensors.h"

#include <utility> // std::pair
#include <vector>
// #include "src/exports/export_struct.h"

namespace Measurements
{
class CCorrelation
{
public:
  struct SResult
  {
    Measurements::SSensors sensor1;
    Measurements::SSensors sensor2;
    double correlation;
  };
  static std::vector<SResult> GetCorrelation(const Measurements::SAllSensors& allSensors,
                                             const std::vector<Measurements::SMeasurementsData>* measuredData,
                                             const bool enablePretestZeroes = false);

  static double GetCorrelationCoefficient(const std::vector<double>& u, const std::vector<double>& v);

private:
  struct SSensorMeasurements
  {
    struct SMeasurement
    {
      Measurements::SMeasuredItem item;
      bool isMeasured = false;
      void AddItem(const Measurements::SMeasuredItem& item_)
      {
        item = item_;
        isMeasured = true;
      }
    };
    std::vector<SMeasurement> rawMeasurements;
    Measurements::SSensors sensor;
  };
  inline static constexpr int minimumCorrelationSize_ = 10;

  static std::pair<SSensorMeasurements, SSensorMeasurements> RemoveNotMeasured(const SSensorMeasurements& vec1,
                                                                               const SSensorMeasurements& vec2);
  static std::pair<std::vector<double>, std::vector<double>> ConvertToRaw(const SSensorMeasurements& vec1,
                                                                          const SSensorMeasurements& vec2);
  static std::vector<double> ConvertToRaw(const SSensorMeasurements& vec1);
  static std::vector<SSensorMeasurements> CreateEqualSizedVectors(
    const Measurements::SAllSensors& allSensors,
    const std::vector<Measurements::SMeasurementsData>* measuredData,
    const bool isPerformanceMetric,
    const bool enablePretestZeroes);
  static std::vector<SSensorMeasurements> GetSensors(const Measurements::SAllSensors& allSensors,
                                                     const Measurements::EClassification classification,
                                                     const bool isPerformanceMetric);
  static std::vector<SSensorMeasurements> GetSensors(const std::vector<SSensors>& sensors,
                                                     const bool isPerformanceMetric);
};

} // namespace Measurements