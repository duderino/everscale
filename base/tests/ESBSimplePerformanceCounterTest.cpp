#ifndef ESB_SIMPLE_PERFORMANCE_COUNTER_H
#include <ESBSimplePerformanceCounter.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(SimplePerformanceCounter, QPS) {
  Date start(Date::Now());
  Date stop(start.seconds(), start.microSeconds() + 1000);
  SimplePerformanceCounter counter("qps-test", start, stop);
  const int queries = 6;

  for (int i = 0; i < queries; ++i) {
    counter.record(start, stop);
  }

  EXPECT_EQ(queries * 1000, counter.queriesPerSec());

  for (int i = 0; i < queries; ++i) {
    counter.record(start, stop);
  }

  EXPECT_EQ(queries * 2000, counter.queriesPerSec());
}

TEST(SimplePerformanceCounter, LatencySec) {
  SimplePerformanceCounter counter("latency-sec-test");
  Date start(Date::Now());
  Date stop(start);
  double totalMsec = 0.0;
  const int queries = 6;

  for (int i = 0; i < queries; ++i) {
    stop += 1;
    totalMsec += 1000 * (i + 1);
    counter.record(start, stop);
  }

  double meanMsec = totalMsec / queries;
  EXPECT_EQ(meanMsec, counter.meanMSec());
  EXPECT_EQ(1000, counter.minMSec());
  EXPECT_EQ(1000 * queries, counter.maxMSec());

  // Calculate Variance in second pass

  stop = start;
  double squaredDistToMeanMsec = 0.0;
  for (int i = 0; i < queries; ++i) {
    stop += 1;
    double latencyMsec = 1000 * (i + 1);
    squaredDistToMeanMsec +=
        (latencyMsec - meanMsec) * (latencyMsec - meanMsec);
  }

  double varianceMsec = squaredDistToMeanMsec / (queries - 1);
  EXPECT_EQ(varianceMsec, counter.varianceMSec());
}

TEST(SimplePerformanceCounter, LatencyMsec) {
  SimplePerformanceCounter counter("latency-msec-test");
  Date start(Date::Now());
  double totalMsec = 0UL;
  const int queries = 6;

  for (uint i = 0; i < queries; ++i) {
    Date micros(0, 1000 * (i + 1));
    Date stop = start + micros;
    totalMsec += micros.microSeconds() / 1000;
    counter.record(start, stop);
  }

  double meanMsec = totalMsec / queries;
  EXPECT_EQ(meanMsec, counter.meanMSec());
  EXPECT_EQ(1, counter.minMSec());
  EXPECT_EQ(queries, counter.maxMSec());

  // Calculate Variance in second pass

  double squaredDistToMeanMsec = 0.0;
  for (int i = 0; i < queries; ++i) {
    Date micros(0, 1000 * (i + 1));
    Date stop = start + micros;
    double latencyMsec = micros.microSeconds() / 1000;
    squaredDistToMeanMsec +=
        (latencyMsec - meanMsec) * (latencyMsec - meanMsec);
  }

  double varianceMsec = squaredDistToMeanMsec / (queries - 1);
  EXPECT_EQ(varianceMsec, counter.varianceMSec());
}
