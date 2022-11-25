#include "correlation.h"

#include <cmath>
#include <set>

namespace Measurements
{
/**
 * @brief Calculate the Correlation
 *
 * What this function does:
 * 1. Create evenly length vectors with a pair <double measurement, bool
 * isMeasured> for all measurements
 * 1.1 Separate these vectors between performance vectors and non-performance
 * vectors
 *
 * 2. Loop through all the vectors and create all combinations of performance &
 * resource usage (to calculate the correlation between them)
 *
 * 3. When having 2 vectors (one performance and one resource usage), remove
 * all measurements where isMeasured=false (also remove the same index in the
 * other vector). The result are equal sized vectors with all isMeasured=true
 * The isMeasured should be true in both vectors because measurements on the
 * same timeframe can be compared (one measurement at T=0s and one at T=1s
 * cannot be reliably compared, as the situation could differ between these
 * timeframes)
 *
 *
 * 4. Execute the correlation function, but only if the vectors have
 * size > minimumCorrelationSize_ (the bigger the size, the more accurate the
 * correlation measurement is, a minimum correlation size of 10 is recommended)
 *
 * 5. Store (and return) the correlation result of -1 <= correlation <= 1
 */
std::vector<CCorrelation::SResult> CCorrelation::GetCorrelation(
  const Measurements::SAllSensors& allSensors,
  const std::vector<Measurements::SMeasurementsData>* measuredData,
  const bool enablePretestZeroes)
{
  auto equalVectorsPerf = CreateEqualSizedVectors(allSensors, measuredData, true, enablePretestZeroes);
  auto equalVectorsRsc = CreateEqualSizedVectors(allSensors, measuredData, false, enablePretestZeroes);
  std::vector<SResult> correlationResults;

  for (const auto& rscVector : equalVectorsRsc)
  {
    for (const auto& perfVector : equalVectorsPerf)
    {
      const auto& [rscVectorCorrected, perfVectorCorrected] = RemoveNotMeasured(rscVector, perfVector);

      const auto& rawRscVec = ConvertToRaw(rscVectorCorrected);
      const auto& rawPerfVec = ConvertToRaw(perfVectorCorrected);
      if (rawRscVec.size() >= minimumCorrelationSize_)
      {
        SResult correlationResult;
        correlationResult.sensor1 = perfVectorCorrected.sensor;
        correlationResult.sensor2 = rscVectorCorrected.sensor;
        correlationResult.correlation = GetCorrelationCoefficient(rawRscVec, rawPerfVec);
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
 */
std::vector<CCorrelation::SSensorMeasurements> CCorrelation::CreateEqualSizedVectors(
  const Measurements::SAllSensors& allSensors,
  const std::vector<Measurements::SMeasurementsData>* measuredData,
  const bool isPerformanceMetric,
  const bool enablePretestZeroes)
{
  std::vector<CCorrelation::SSensorMeasurements> allCorrelations;

  for (const auto& classification : allSensors.allClasses)
  {
    for (const auto& sensorGroup : allSensors.GetSensorGroups(classification))
    {
      auto equalSizedVector = GetSensors(sensorGroup.sensors, isPerformanceMetric);

      // Loop through all the sensors and add all the values
      for (auto& sensor : equalSizedVector)
      {
        // Loop through all the raw measurements and add them to the correct
        // sensor
        for (auto& measurement : *measuredData)
        {
          SSensorMeasurements::SMeasurement resMeasurement;
          auto measurementGroup = measurement.GetItems(classification);
          // Loop through all the measured fields and find the one equal to sensor
          for (const auto& field : measurementGroup)
          {
            if (field.id == sensor.sensor.uniqueId)
            {
              resMeasurement.AddItem(field);
              break;
            }
          }
          if (enablePretestZeroes && !resMeasurement.isMeasured)
          {
            if (std::stoll(measurement.time) < sensorGroup.processDelay)
            {
              resMeasurement.isMeasured = true;
              resMeasurement.item.measuredValue = 0;
            }
          }
          sensor.rawMeasurements.push_back(resMeasurement);
        }
      }

      allCorrelations = Helpers::CombineVectors(allCorrelations, equalSizedVector);
    }
  }
  return allCorrelations;
}

std::vector<CCorrelation::SSensorMeasurements> CCorrelation::GetSensors(
  const Measurements::SAllSensors& allSensors,
  const Measurements::EClassification classification,
  const bool isPerformanceMetric)
{
  auto sensors = allSensors.GetSensors(classification);
  return GetSensors(sensors, isPerformanceMetric);
}

std::vector<CCorrelation::SSensorMeasurements> CCorrelation::GetSensors(const std::vector<SSensors>& sensors,
                                                                        const bool isPerformanceMetric)
{
  std::vector<SSensorMeasurements> result;
  for (const auto& sensor : sensors)
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
double CCorrelation::GetCorrelationCoefficient(const std::vector<double>& arrU, const std::vector<double>& arrV)
{
  if (arrU.size() != arrV.size())
  {
    throw std::runtime_error("Correlation failed! Vectors are not equal length!");
  }

  double sum_U = 0, sum_V = 0, sum_UV = 0;
  double squareSum_U = 0, squareSum_V = 0;

  size_t arrLength = arrU.size();
  for (size_t i = 0; i < arrLength; i++)
  {
    auto u = arrU.at(i);
    auto v = arrV.at(i);
    // sum of elements of array X.
    sum_U += u;

    // sum of elements of array Y.
    sum_V += v;

    // sum of X[i] * Y[i].
    sum_UV += (u * v);

    // sum of square of array elements.
    squareSum_U += (u * u);
    squareSum_V += (v * v);
  }
  // use formula for calculating correlation coefficient.
  double corr = ((arrLength * sum_UV) - (sum_U * sum_V)) /
                sqrt(((arrLength * squareSum_U) - (sum_U * sum_U)) * ((arrLength * squareSum_V) - (sum_V * sum_V)));

  return corr;
}

/**
 * @brief Removes the fields that are not measured from the vectors
 */
std::pair<CCorrelation::SSensorMeasurements, CCorrelation::SSensorMeasurements> CCorrelation::RemoveNotMeasured(
  const SSensorMeasurements& vec1,
  const SSensorMeasurements& vec2)
{
  auto GetRemoveIndexes = [](const SSensorMeasurements& vec) {
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

  auto RemoveIndexes = [](const SSensorMeasurements& vec, const std::set<int, std::greater<int>>& indexes) {
    auto vecCpy = vec.rawMeasurements;
    // Start with removal of the largest index, therefore the set is ordered with std::greater
    for (const auto& index : indexes)
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

std::vector<double> CCorrelation::ConvertToRaw(const SSensorMeasurements& vec1)
{
  std::vector<double> result;

  std::transform(vec1.rawMeasurements.begin(),
                 vec1.rawMeasurements.end(),
                 std::back_inserter(result),
                 [](const auto& e) { return e.item.measuredValue; });

  return result;
}

} // namespace Measurements