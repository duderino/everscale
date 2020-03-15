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
  Date stop(start.getSeconds(), start.getMicroSeconds() + 1000);

  for (int i = 0; i < queries; ++i) {
    timeSeries.addObservation(start, stop);
  }

  EXPECT_EQ(timeSeries.getCounters()->length(), 1);
  SimplePerformanceCounter *first =
      (SimplePerformanceCounter *)timeSeries.getCounters()->getFirst();
  EXPECT_EQ(first->getQueries(), queries);
}

TEST(TimeSeries, MultipleWindows) {
  const UInt16 queries = 6;
  const UInt16 maxWindows = 2;
  const UInt16 windowSizeSec = 10;
  TimeSeries timeSeries("ts-test", maxWindows, windowSizeSec);
  Date start(Date::Now());
  Date stop(start + 1);

  for (int i = 0; i < queries; ++i) {
    timeSeries.addObservation(start, stop);
  }

  EXPECT_EQ(timeSeries.getCounters()->length(), 1);
  SimplePerformanceCounter *first =
      (SimplePerformanceCounter *)timeSeries.getCounters()->getFirst();
  EXPECT_EQ(first->getQueries(), queries);

  stop += windowSizeSec;

  for (int i = 0; i < queries; ++i) {
    timeSeries.addObservation(start, stop);
  }

  EXPECT_EQ(timeSeries.getCounters()->length(), 2);
  first = (SimplePerformanceCounter *)timeSeries.getCounters()->getFirst();
  EXPECT_EQ(first->getQueries(), queries);
  SimplePerformanceCounter *second =
      (SimplePerformanceCounter *)first->getNext();
  EXPECT_EQ(second->getQueries(), queries);
}

TEST(TimeSeries, ExceedMaxWindows) {
  const UInt16 queries = 6;
  const UInt16 maxWindows = 1;
  const UInt16 windowSizeSec = 10;
  TimeSeries timeSeries("ts-test", maxWindows, windowSizeSec);
  Date start(Date::Now());
  Date stop(start + 1);

  for (int i = 0; i < queries; ++i) {
    timeSeries.addObservation(start, stop);
  }

  EXPECT_EQ(timeSeries.getCounters()->length(), 1);
  SimplePerformanceCounter *first =
      (SimplePerformanceCounter *)timeSeries.getCounters()->getFirst();
  EXPECT_EQ(first->getQueries(), queries);

  stop += windowSizeSec;

  for (int i = 0; i < queries; ++i) {
    timeSeries.addObservation(start, stop);
  }

  EXPECT_EQ(timeSeries.getCounters()->length(), 1);
  first = (SimplePerformanceCounter *)timeSeries.getCounters()->getFirst();
  EXPECT_EQ(first->getQueries(), queries);
}