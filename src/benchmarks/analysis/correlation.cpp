#include "correlation.h"

#include <set>

#if BOOST_VERSION / 100 >= 1072
#include <boost/math/statistics/bivariate_statistics.hpp>
#else
#include <boost/math/tools/bivariate_statistics.hpp>
#endif

namespace Measurements
{
/**
 * @brief Get the Correlation object
 *
 * @param allSensors
 * @param measuredData
 * Steps to execute:
 * 1. Create evenly length vectors with a pair <double measurement, bool
 * isMeasured> for all measurements
 * 1.1 Separate these vectors between performance vectors and non-performance
 * vectors
 *
 *  2. Loop through all the vectors and create all combinations of performance &
 * resource usage
 *
 *  3. When having 2 vectors (one performance and one resource usage), remove
 * all measurements where isMeasured=false (also remove the same index in the
 * other vector). The result are equal sized vectors with all isMeasurement=true
 *
 *  4. Execute the correlation function, but only if the vectors have size>1
 * (size=1 is useless)
 *
 *  5. Store the correlation result of -1 <= correlation <= 1
 */
std::vector<CCorrelation::SResult> CCorrelation::GetCorrelation(
    const Measurements::AllSensors &allSensors,
    const std::vector<Measurements::SMeasurementsData> *measuredData)
{
  auto equalVectorsPerf =
      CreateEqualSizedVectors(allSensors, measuredData, true);
  auto equalVectorsRsc =
      CreateEqualSizedVectors(allSensors, measuredData, false);
  std::vector<SResult> correlationResults;

  for (const auto &rscVector : equalVectorsRsc)
  {
    for (const auto &perfVector : equalVectorsPerf)
    {
      const auto &[rscVectorCorrected, perfVectorCorrected] =
          RemoveNotMeasured(rscVector, perfVector);

      const auto &rawRscVec = ConvertToRaw(rscVectorCorrected);
      const auto &rawPerfVec = ConvertToRaw(perfVectorCorrected);
      if (rawRscVec.size() >= 10)
      {
        SResult correlationResult;
        correlationResult.sensor1 = perfVectorCorrected.sensor;
        correlationResult.sensor2 = rscVectorCorrected.sensor;
        correlationResult.correlation =
            GetCorrelationCoefficient(rawRscVec, rawPerfVec);
        correlationResults.push_back(correlationResult);
      }
    }
  }
  return correlationResults;
}

/**
 * @brief Create a Equal Sized Vectors object
 *
 * This function performs the following tasks:
 * 1. Loop through all measurements
 * 2. Make a vector for each measurement
 *
 * @TODO Remove duplicate code by fixing the Exports::ExportData structure!
 */
std::vector<CCorrelation::SSensorMeasurements>
CCorrelation::CreateEqualSizedVectors(
    const Measurements::AllSensors &allSensors,
    const std::vector<Measurements::SMeasurementsData> *measuredData,
    const bool isPerformanceMetric)
{
  std::vector<CCorrelation::SSensorMeasurements> allCorrelations;

  for (const auto &classification : allSensors.allClasses)
  {
    auto equalSizedVector =
        GetSensors(allSensors, classification, isPerformanceMetric);

    // Loop through all the sensors and add all the values
    for (auto &sensor : equalSizedVector)
    {
      // Loop through all the raw measurements and add them to the correct
      // sensor
      for (auto &measurement : *measuredData)
      {
        SSensorMeasurements::SMeasurement resMeasurement;
        auto measurementGroup = measurement.GetItems(classification);
        // Loop through all the measured fields and find the one equal to
        // sensor
        for (const auto &field : measurementGroup)
        {
          if (field.id == sensor.sensor.uniqueId)
          {
            resMeasurement.AddItem(field);
            break;
          }
        }
        sensor.rawMeasurements.push_back(resMeasurement);
      }
    }
    allCorrelations =
        Helpers::CombineVectors(allCorrelations, equalSizedVector);
  }

  // resultPipeline = CreateEqualSizedVector(
  //     allSensors.GetSensorGroups(Classification::PIPELINE), measuredData,
  //     isPerformanceMetric);
  // Loop through all the sensors and add all the values
  // for (auto &sensor : resultPipeline)
  // {
  //   // Loop through all the raw measurements and add them to the correct
  //   sensor for (auto &measurement : *measuredData)
  //   {
  //     SSensorMeasurements::SMeasurement resMeasurement;
  //     // Loop through all the measured fields and find the one equal to
  //     // sensor
  //     for (const auto &pipeline : measurement.pipelineInfo)
  //     {
  //       for (const auto &field : pipeline.measuredItems)
  //       {
  //         if (field.id == sensor.sensor.uniqueId)
  //           resMeasurement.AddItem(field);
  //       }
  //       // Stop checking the other pipelines if found
  //       if (resMeasurement.isMeasured)
  //         break;
  //     }
  //     if (!resMeasurement.isMeasured)
  //     {
  //       // TODO add the not started functionality here
  //       // if (measurement.time < resultPipeline)
  //     }
  //     sensor.rawMeasurements.push_back(resMeasurement);
  //   }
  // }
  return allCorrelations;
}

std::vector<CCorrelation::SSensorMeasurements>
CCorrelation::GetSensors(const Measurements::AllSensors &allSensors,
                         const Measurements::Classification classification,
                         const bool isPerformanceMetric)
{
  auto sensors = allSensors.GetSensors(classification);
  return GetSensors(sensors, isPerformanceMetric);
}

std::vector<CCorrelation::SSensorMeasurements>
CCorrelation::GetSensors(const std::vector<Sensors> &sensors,
                         const bool isPerformanceMetric)
{
  std::vector<SSensorMeasurements> result;
  for (const auto &sensor : sensors)
  {
    if (sensor.performanceIndicator == isPerformanceMetric)
    {
      SSensorMeasurements sensorMeasurement;
      sensorMeasurement.sensor = sensor;
      result.push_back(sensorMeasurement);
    }
  }
  return result;
}

// std::vector<CCorrelation::SSensorMeasurements>
// CCorrelation::CreateEqualSizedVector(
//     const std::vector<Measurements::AllSensors::SensorGroups> &sensorGroups,
//     const std::vector<Measurements::SMeasurementsData> *measuredData,
//     const bool isPerformanceMetric)
// {
//   std::vector<CCorrelation::SSensorMeasurements> result;
//   // Loop through all the sensors and add all the values
//   for (auto &sensorGroup : sensorGroups)
//   {
//     std::vector<CCorrelation::SSensorMeasurements> sensors =
//         GetSensors(sensorGroup.sensors, isPerformanceMetric);
//     // Loop through all the raw measurements and add them to the correct
//     // sensor
//     for (auto &sensor : sensors)
//     {
//       for (auto &measurement : *measuredData)
//       {
//         SSensorMeasurements::SMeasurement resMeasurement;
//         // Loop through all the measured fields and find the one equal to
//         // sensor
//         for (const auto &pipeline : measurement.pipelineInfo)
//         {
//           for (const auto &field : pipeline.measuredItems)
//           {
//             if (field.id == sensor.sensor.uniqueId)
//               resMeasurement.AddItem(field);
//           }
//           // Stop checking the other pipelines if found
//           if (resMeasurement.isMeasured)
//             break;
//         }
//         // if (!resMeasurement.isMeasured)
//         // {
//         //   // TODO add the not started functionality here
//         //   if (std::stoll(measurement.time) < sensorGroup.processDelay)
//         //   {
//         //     resMeasurement.isMeasured = true;
//         //     resMeasurement.item.measuredValue = 0;
//         //   }
//         // }
//         sensor.rawMeasurements.push_back(resMeasurement);
//       }
//     }
//     result = Helpers::CombineVectors(result, sensors);
//   }
//   return result;
// }

/**
 * @brief Calculates the Pearson correlation coefficient
 *
 * @param u the vector which is checked for correlation, size must be equal to
 * vector v
 * @param v the vector which is checked for correlation, size must be equal to
 * vector u
 * @return [-1]: Inversed correlation
 *          [0]: No correlation at all
 *          [1]: A perfect correlation
 * The return value (RetVal) is within: -1 <= RetVal <= 1
 * as a fraction. The higher the value, the more correlated the data is or the
 * lower the value, the higher the inversed correlation is
 */
double CCorrelation::GetCorrelationCoefficient(const std::vector<double> &u,
                                               const std::vector<double> &v)
{
#if BOOST_VERSION / 100 >= 1072
  return boost::math::statistics::correlation_coefficient(u, v);
#else
  return boost::math::tools::correlation_coefficient(u, v);
#endif
}

std::pair<CCorrelation::SSensorMeasurements, CCorrelation::SSensorMeasurements>
CCorrelation::RemoveNotMeasured(const SSensorMeasurements &vec1,
                                const SSensorMeasurements &vec2)
{
  auto GetRemoveIndexes = [](const SSensorMeasurements &vec) {
    std::set<int, std::greater<int>> indexesToBeRemoved;
    for (size_t i = 0; i < vec.rawMeasurements.size(); ++i)
    {
      auto meas = vec.rawMeasurements.at(i);
      if (!meas.isMeasured)
        indexesToBeRemoved.insert(i);
    }
    return indexesToBeRemoved;
  };
  auto indexesToRemove = GetRemoveIndexes(vec1);
  indexesToRemove.merge(GetRemoveIndexes(vec2));

  auto RemoveIndexes = [](const SSensorMeasurements &vec,
                          const std::set<int, std::greater<int>> &indexes) {
    auto vecCpy = vec.rawMeasurements;
    // Start with removal of the largest index, therefore the set is ordered
    // with std::greater
    for (const auto &index : indexes)
    {
      auto deleteIndex = vecCpy.begin() + index;
      vecCpy.erase(deleteIndex);
    }
    auto result = vec;
    result.rawMeasurements = vecCpy;
    return result;
  };
  auto vec1rm = RemoveIndexes(vec1, indexesToRemove);
  auto vec2rm = RemoveIndexes(vec2, indexesToRemove);
  return std::make_pair(vec1rm, vec2rm);
}

std::vector<double> CCorrelation::ConvertToRaw(const SSensorMeasurements &vec1)
{
  std::vector<double> result;
  for (const auto &e : vec1.rawMeasurements)
  {
    result.push_back(e.item.measuredValue);
  }
  return result;
}

} // namespace Measurements