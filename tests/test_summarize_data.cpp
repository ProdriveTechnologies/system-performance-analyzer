#include <gtest/gtest.h>

#include "src/benchmarks/linux/summarize_data.h"

// Demonstrate some basic assertions.
TEST(SummarizeData, EqualCheck)
{
  Measurements::CSummarizeData summarize;
  summarize.AddDataPoint(Exports::SMeasuredItem{3, 6});
  auto sensordata = summarize.GetSensorData();

  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::AVERAGE), 6);
  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MEDIAN), 6);
  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MIN), 6);
  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MAX), 6);
}

TEST(SummarizeData, Average)
{
  Measurements::CSummarizeData summarize;
  summarize.AddDataPoint(Exports::SMeasuredItem{3, 6});
  summarize.AddDataPoint(Exports::SMeasuredItem{3, 12});
  auto sensordata = summarize.GetSensorData();

  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::AVERAGE), 9.0);
  summarize.AddDataPoint(Exports::SMeasuredItem{3, 0});
  sensordata = summarize.GetSensorData();
  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::AVERAGE), 6.0);
}

TEST(SummarizeData, Minimum)
{
  Measurements::CSummarizeData summarize;
  summarize.AddDataPoint(Exports::SMeasuredItem{3, 6});
  auto sensordata = summarize.GetSensorData();
  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MIN), 6.0);

  summarize.AddDataPoint(Exports::SMeasuredItem{3, 12});
  sensordata = summarize.GetSensorData();

  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MIN), 6.0);
  summarize.AddDataPoint(Exports::SMeasuredItem{3, 0});
  sensordata = summarize.GetSensorData();
  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MIN), 0.0);
}

TEST(SummarizeData, Maximum)
{
  Measurements::CSummarizeData summarize;
  summarize.AddDataPoint(Exports::SMeasuredItem{3, 6});
  auto sensordata = summarize.GetSensorData();
  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MAX), 6.0);

  summarize.AddDataPoint(Exports::SMeasuredItem{3, 12});
  sensordata = summarize.GetSensorData();

  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MAX), 12.0);
  summarize.AddDataPoint(Exports::SMeasuredItem{3, 0});
  sensordata = summarize.GetSensorData();
  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MAX), 12.0);
}

TEST(SummarizeData, Median)
{
  Measurements::CSummarizeData summarize;
  summarize.AddDataPoint(Exports::SMeasuredItem{3, 6});
  auto sensordata = summarize.GetSensorData();
  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MEDIAN), 6.0);

  summarize.AddDataPoint(Exports::SMeasuredItem{3, 12});
  sensordata = summarize.GetSensorData();

  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MEDIAN), 9.0);
  summarize.AddDataPoint(Exports::SMeasuredItem{3, 0});
  sensordata = summarize.GetSensorData();
  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MEDIAN), 6.0);

  summarize.AddDataPoint(Exports::SMeasuredItem{3, 20});
  sensordata = summarize.GetSensorData();
  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MEDIAN), 9.0);
}

TEST(SummarizeData, Multiplier)
{
  Measurements::CSummarizeData summarize;
  summarize.SetMultiplier(5.0);
  summarize.AddDataPoint(Exports::SMeasuredItem{3, 6});
  auto sensordata = summarize.GetSensorData();

  auto result = 6 * 5.0;

  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::AVERAGE), result);
  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MEDIAN), result);
  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MIN), result);
  EXPECT_EQ(sensordata.Get(Measurements::ValueTypes::MAX), result);
}