#ifndef ESB_TIME_H
#include <ESBTime.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(Time, MonotonicTime) {
  Error error = Time::Instance().start();
  EXPECT_EQ(ESB_SUCCESS, error);

  Date first = Time::Instance().now();
  sleep(2);
  Date second = Time::Instance().now();

  EXPECT_GE(2, (second - first).seconds());

  Time::Instance().stop();
  error = Time::Instance().join();
  EXPECT_EQ(ESB_SUCCESS, error);
}

TEST(Time, Uninitialized) {
  // don't segfault if the background thread isn't started
  Date first = Time::Instance().now();
  Date second = Time::Instance().now();
  EXPECT_EQ(second, first);
}
