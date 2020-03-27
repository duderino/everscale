#ifndef ESB_TIME_SERIES_H
#include <ESBTimeSeries.h>
#endif

#ifndef ESB_SIMPLE_PERFORMANCE_COUNTER_H
#include <ESBSimplePerformanceCounter.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(TimeSeries, Basic) {
  const UInt16 queries = 6;
  const UInt16 maxWindows = 1;
  const UInt16 windowSizeSec = 1;
  TimeSeries timeSeries("ts-test", maxWindows, windowSizeSec);
  Date start(Date::Now());
  Date stop(start.seconds(), start.microSeconds() + 1000);

  for (int i = 0; i < queries; ++i) {
    timeSeries.record(start, stop);
  }

  EXPECT_EQ(timeSeries.counters()->size(), 1);
  SimplePerformanceCounter *first =
      (SimplePerformanceCounter *)timeSeries.counters()->first();
  EXPECT_EQ(first->queries(), queries);
}

TEST(TimeSeries, MultipleWindows) {
  const UInt16 queries = 6;
  const UInt16 maxWindows = 2;
  const UInt16 windowSizeSec = 10;
  TimeSeries timeSeries("ts-test", maxWindows, windowSizeSec);
  Date start(Date::Now());
  Date stop(start + 1);

  for (int i = 0; i < queries; ++i) {
    timeSeries.record(start, stop);
  }

  EXPECT_EQ(timeSeries.counters()->size(), 1);
  SimplePerformanceCounter *first =
      (SimplePerformanceCounter *)timeSeries.counters()->first();
  EXPECT_EQ(first->queries(), queries);

  stop += windowSizeSec;

  for (int i = 0; i < queries; ++i) {
    timeSeries.record(start, stop);
  }

  EXPECT_EQ(timeSeries.counters()->size(), 2);
  first = (SimplePerformanceCounter *)timeSeries.counters()->first();
  EXPECT_EQ(first->queries(), queries);
  SimplePerformanceCounter *second = (SimplePerformanceCounter *)first->next();
  EXPECT_EQ(second->queries(), queries);
}

TEST(TimeSeries, ExceedMaxWindows) {
  const UInt16 queries = 6;
  const UInt16 maxWindows = 1;
  const UInt16 windowSizeSec = 10;
  TimeSeries timeSeries("ts-test", maxWindows, windowSizeSec);
  Date start(Date::Now());
  Date stop(start + 1);

  for (int i = 0; i < queries; ++i) {
    timeSeries.record(start, stop);
  }

  EXPECT_EQ(timeSeries.counters()->size(), 1);
  SimplePerformanceCounter *first =
      (SimplePerformanceCounter *)timeSeries.counters()->first();
  EXPECT_EQ(first->queries(), queries);

  stop += windowSizeSec;

  for (int i = 0; i < queries; ++i) {
    timeSeries.record(start, stop);
  }

  EXPECT_EQ(timeSeries.counters()->size(), 1);
  first = (SimplePerformanceCounter *)timeSeries.counters()->first();
  EXPECT_EQ(first->queries(), queries);
}