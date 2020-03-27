#ifndef ESB_AVERAGING_COUNTER_H
#include <ESBAveragingCounter.h>
#endif

#ifndef ESB_SHARED_AVERAGING_COUNTER_H
#include <ESBSharedAveragingCounter.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(AveragingCounter, MeanAndVariance) {
  AveragingCounter counter;
  double total = 0.0;
  UInt32 observations = 6;

  for (UInt32 i = 0; i < observations; ++i) {
    double value = 1000 * (i + 1);
    total += value;
    counter.add(value);
  }

  double mean = total / observations;
  EXPECT_EQ(mean, counter.mean());
  EXPECT_EQ(1000, counter.min());
  EXPECT_EQ(1000 * observations, counter.max());

  // Calculate Variance in second pass

  double squaredDistToMean = 0.0;

  for (UInt32 i = 0; i < observations; ++i) {
    double value = 1000 * (i + 1);
    squaredDistToMean += (value - mean) * (value - mean);
  }

  double variance = squaredDistToMean / (observations - 1);
  EXPECT_EQ(variance, counter.variance());
}

TEST(SharedAveragingCounter, MeanAndVariance) {
  SharedAveragingCounter counter;
  double total = 0.0;
  UInt32 observations = 6;

  for (UInt32 i = 0; i < observations; ++i) {
    double value = 1000 * (i + 1);
    total += value;
    counter.add(value);
  }

  double mean = total / observations;
  EXPECT_EQ(mean, counter.mean());
  EXPECT_EQ(1000, counter.min());
  EXPECT_EQ(1000 * observations, counter.max());

  // Calculate Variance in second pass

  double squaredDistToMean = 0.0;

  for (UInt32 i = 0; i < observations; ++i) {
    double value = 1000 * (i + 1);
    squaredDistToMean += (value - mean) * (value - mean);
  }

  double variance = squaredDistToMean / (observations - 1);
  EXPECT_EQ(variance, counter.variance());
}
