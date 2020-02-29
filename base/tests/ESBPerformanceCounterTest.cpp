#ifndef ESB_SIMPLE_PERFORMANCE_COUNTER_H
#include <ESBSimplePerformanceCounter.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(SimplePerformanceCounter, QPS) {
  SimplePerformanceCounter counter("qps-test");
  Date start(Date::Now());
  Date stop(start);
  stop += 1;
  int queries = 6;

  for (int i = 0; i < queries; ++i) {
    counter.addObservation(start, stop);
  }

  EXPECT_EQ(queries, counter.getQueriesPerSec());

  for (int i = 0; i < queries; ++i) {
    counter.addObservation(start, stop);
  }

  EXPECT_EQ(queries * 2, counter.getQueriesPerSec());
}

TEST(SimplePerformanceCounter, LatencySec) {
  SimplePerformanceCounter counter("latency-sec-test");
  Date start(Date::Now());
  Date stop(start);
  double totalMsec = 0UL;
  int queries = 6;

  for (int i = 0; i < queries; ++i) {
    stop += 1;
    totalMsec += 1000 * (i + 1);
    counter.addObservation(start, stop);
  }

  EXPECT_EQ(totalMsec / queries, counter.getAvgMsec());
  EXPECT_EQ(1000, counter.getMinMsec());
  EXPECT_EQ(1000 * queries, counter.getMaxMsec());
}

TEST(SimplePerformanceCounter, LatencyMsec) {
  SimplePerformanceCounter counter("latency-msec-test");
  Date start(Date::Now());
  Date stop(start);
  Date msec10(0, 10);
  double totalMsec = 0UL;
  int queries = 6;

  for (int i = 0; i < queries; ++i) {
    Date tmp(stop.getSeconds(), stop.getMicroSeconds() + 10);
    stop = tmp;
    // stop += msec10;
    totalMsec += 10 * (i + 1);
    counter.addObservation(start, stop);
  }

  EXPECT_EQ(totalMsec / queries, counter.getAvgMsec());
  EXPECT_EQ(10, counter.getMinMsec());
  EXPECT_EQ(10 * queries, counter.getMaxMsec());
}
