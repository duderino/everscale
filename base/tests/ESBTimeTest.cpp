#ifndef ESB_TIME_H
#include <ESBTime.h>
#endif

#ifndef ESB_TIME_SOURCE_CACHE_H
#include <ESBTimeSourceCache.h>
#endif

#ifndef ESB_SYSTEM_TIME_SOURCE_H
#include <ESBSystemTimeSource.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(Time, MonotonicTime) {
  Date first = Time::Instance().now();
  usleep(1000);
  Date second = Time::Instance().now();

  EXPECT_LT(first, second);
}

TEST(Time, CachedMonotonicTime) {
  TimeSourceCache timeSourceCache(SystemTimeSource::Instance());
  Time::Instance().setTimeSource(timeSourceCache);

  Error error = timeSourceCache.start();
  EXPECT_EQ(ESB_SUCCESS, error);

  Date first = Time::Instance().now();
  sleep(2);
  Date second = Time::Instance().now();
  EXPECT_GE(2, (second - first).seconds());

  Time::Instance().setTimeSource(SystemTimeSource::Instance());
  timeSourceCache.stop();
  error = timeSourceCache.join();
  EXPECT_EQ(ESB_SUCCESS, error);
}
