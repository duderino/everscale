#ifndef ESB_SIMPLE_PERFORMANCE_COUNTER_H
#include <ESBSimplePerformanceCounter.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(SimplePerformanceCounter, QPS) {
  Date start(Date::Now());
  Date stop(start.getSeconds(), start.getMicroSeconds() + 1000);
  SimplePerformanceCounter counter("qps-test", start, stop);
  const int queries = 6;

  for (int i = 0; i < queries; ++i) {
    counter.addObservation(start, stop);
  }

  EXPECT_EQ(queries * 1000, counter.getQueriesPerSec());

  for (int i = 0; i < queries; ++i) {
    counter.addObservation(start, stop);
  }

  EXPECT_EQ(queries * 2000, counter.getQueriesPerSec());
}

TEST(SimplePerformanceCounter, LatencySec) {
  SimplePerformanceCounter counter("latency-sec-test");
  Date start(Date::Now());
  Date stop(start);
  double totalMsec = 0UL;
  const int queries = 6;

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
  double totalMsec = 0UL;
  const int queries = 6;

  for (uint i = 0; i < queries; ++i) {
    Date micros(0, 1000 * (i + 1));
    Date stop = start + micros;
    totalMsec += micros.getMicroSeconds() / 1000;
    counter.addObservation(start, stop);
  }

  EXPECT_EQ(totalMsec / queries, counter.getAvgMsec());
  EXPECT_EQ(1, counter.getMinMsec());
  EXPECT_EQ(queries, counter.getMaxMsec());
}
