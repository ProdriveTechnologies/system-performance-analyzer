#pragma once

#include <utility> // std::pair
#include <vector>

#include "src/benchmarks/linux/struct_sensors.h"
#include "src/exports/export_struct.h"

namespace Measurements
{
class CCorrelation
{
public:
  struct SResult
  {
    Measurements::Sensors sensor1;
    Measurements::Sensors sensor2;
    double correlation;
  };
  static std::vector<SResult>
  GetCorrelation(const Measurements::AllSensors &allSensors,
                 const std::vector<Exports::ExportData> *measuredData);

  static double GetCorrelationCoefficient(const std::vector<double> &u,
                                          const std::vector<double> &v);

private:
  struct SSensorMeasurements
  {
    struct SMeasurement
    {
      Exports::MeasuredItem item;
      bool isMeasured = false;
      void AddItem(const Exports::MeasuredItem &item_)
      {
        item = item_;
        isMeasured = true;
      }
    };
    std::vector<SMeasurement> rawMeasurements;
    Measurements::Sensors sensor;
  };

  static std::pair<SSensorMeasurements, SSensorMeasurements>
  RemoveNotMeasured(const SSensorMeasurements &vec1,
                    const SSensorMeasurements &vec2);
  static std::pair<std::vector<double>, std::vector<double>>
  ConvertToRaw(const SSensorMeasurements &vec1,
               const SSensorMeasurements &vec2);
  static std::vector<double> ConvertToRaw(const SSensorMeasurements &vec1);
  static std::vector<SSensorMeasurements>
  CreateEqualSizedVectors(const Measurements::AllSensors &allSensors,
                          const std::vector<Exports::ExportData> *measuredData,
                          const bool isPerformanceMetric);
  static std::vector<SSensorMeasurements>
  GetSensors(const Measurements::AllSensors &allSensors,
             const Measurements::Classification classification,
             const bool isPerformanceMetric);
};

} // namespace Measurements