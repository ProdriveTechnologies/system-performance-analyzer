#include <gtest/gtest.h>

#include "src/benchmarks/analysis/correlation.h"

/**
 * @brief Test the correlation function for perfectly correlated vectors
 *
 */
TEST(Correlation, Correlated)
{
  Measurements::CCorrelation correlation;
  std::vector<double> data{1, 2, 3, 4, 5, 4, 4, 2};
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
  std::vector<double> data{1, 2, 3, 4, 5, 4, 4, 2};
  auto negatedData = data;
  std::for_each(negatedData.begin(), negatedData.end(),
                [](auto &e) { e = -e; });

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
  std::vector<double> data{1, 2, 3, 4, 5, 4, 4, 2};
  std::vector<double> data2{1, 1, -1, 3, 1, 3, 0, 6};

  auto res = correlation.GetCorrelationCoefficient(data, data2);
  EXPECT_GT(res, -0.5);
  EXPECT_LT(res, 0.5);
}