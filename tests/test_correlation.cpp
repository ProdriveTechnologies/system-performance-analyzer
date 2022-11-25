#include "src/benchmarks/analysis/correlation.h"

#include <gtest/gtest.h>

void AddPoints(std::vector<Measurements::SMeasurementsData>* alldata,
               Measurements::SMeasuredItem item1,
               Measurements::SMeasuredItem item2 = Measurements::SMeasuredItem{});
/**
 * @brief Test the correlation function for perfectly correlated vectors
 *
 */
TEST(Correlation, Correlated)
{
  Measurements::CCorrelation correlation;
  std::vector<double> data{ 1, 2, 3, 4, 5, 4, 4, 2 };
  auto res = correlation.GetCorrelationCoefficient(data, data);

  // Expect equality.
  EXPECT_EQ(res, 1.0);
}

/**
 * @brief Test the correlation function for perfectly inverse correlated vectors
 *
 */
TEST(Correlation, InverseCorrelated)
{
  Measurements::CCorrelation correlation;
  std::vector<double> data{ 1, 2, 3, 4, 5, 4, 4, 2 };
  auto negatedData = data;
  std::for_each(negatedData.begin(), negatedData.end(), [](auto& e) { e = -e; });

  auto res = correlation.GetCorrelationCoefficient(data, negatedData);
  EXPECT_EQ(res, -1.0);
}

/**
 * @brief Test the correlation function for non/barely correlated vectors
 *
 */
TEST(Correlation, LowCorrelation)
{
  Measurements::CCorrelation correlation;
  std::vector<double> data{ 1, 2, 3, 4, 5, 4, 4, 2 };
  std::vector<double> data2{ 1, 1, -1, 3, 1, 3, 0, 6 };

  auto res = correlation.GetCorrelationCoefficient(data, data2);
  EXPECT_GT(res, -0.5);
  EXPECT_LT(res, 0.5);
}

/**
 * @brief Test the correlation function for non/barely correlated vectors
 *
 */
TEST(Correlation, CorrelationFull)
{
  Measurements::SAllSensors allsensors;
  Measurements::SSensors sensor{ "name", 1, PlatformConfig::EClass::PIPELINE_MEASUREMENTS };
  sensor.performanceIndicator = true;
  Measurements::SSensors sensor2{ "name", 2 };
  sensor2.performanceIndicator = false;

  std::vector<Measurements::SMeasurementsData> alldata;
  using _Item = Measurements::SMeasuredItem;

  for (size_t i = 0; i < 25; i++)
  {
    auto i_d = static_cast<double>(i);
    // Remove half of the values for the 2nd item, to see whether the
    // correlation function can handle unequal vector lengths
    if (i % 2 == 0)
      AddPoints(&alldata, _Item{ 1, i_d }, _Item{ 2, i_d * 3 });
    else
      AddPoints(&alldata, _Item{ 1, i_d });
  }

  allsensors.AddSensors(Measurements::EClassification::PIPELINE, { sensor, sensor2 });

  auto res = Measurements::CCorrelation::GetCorrelation(allsensors, &alldata);

  EXPECT_EQ(res.size(), 1);
  EXPECT_EQ(res.at(0).correlation, 1);
}

void AddPoints(std::vector<Measurements::SMeasurementsData>* alldata,
               Measurements::SMeasuredItem item1,
               Measurements::SMeasuredItem item2)
{
  using _Item = Measurements::SMeasuredItem;
  Measurements::SMeasurementsData data;
  data.time = "3000";
  if (item2.id != -1)
    data.AddMeasurements(Measurements::EClassification::PIPELINE, { item1, item2 });
  else
    data.AddMeasurements(Measurements::EClassification::PIPELINE, { item1 });
  alldata->push_back(data);
}