#ifndef ESB_DATE_H
#include <ESBDate.h>
#endif

#ifndef ESB_TIME_H
#include <ESBTime.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(Date, SecondAddition) {
  const Date second(1, 0);
  const Date orig(Time::Instance().now());
  Date var(orig);

  var += 1;

  EXPECT_EQ(orig.seconds() + 1, var.seconds());
  EXPECT_EQ(orig.microSeconds(), var.microSeconds());

  var = orig;
  var += second;

  EXPECT_EQ(orig.seconds() + 1, var.seconds());
  EXPECT_EQ(orig.microSeconds(), var.microSeconds());

  var = orig + second;

  EXPECT_EQ(orig.seconds() + 1, var.seconds());
  EXPECT_EQ(orig.microSeconds(), var.microSeconds());
}

TEST(Date, MicroSecondAddition) {
  const Date microsecond(0, 1);
  const Date orig(Time::Instance().now());
  Date var(orig);

  var = orig;
  var += microsecond;

  EXPECT_EQ(orig.seconds(), var.seconds());
  EXPECT_EQ(orig.microSeconds() + 1, var.microSeconds());

  var = orig + microsecond;

  EXPECT_EQ(orig.seconds(), var.seconds());
  EXPECT_EQ(orig.microSeconds() + 1, var.microSeconds());
}

TEST(Date, MicroSecondAdditionWrap) {
  const Date nearSecond(0, 999999);
  const Date orig(Time::Instance().now());
  Date var(orig);

  var = orig;
  var += nearSecond;

  EXPECT_EQ(orig.seconds() + 1, var.seconds());
  EXPECT_EQ(orig.microSeconds() - 1, var.microSeconds());

  var = orig + nearSecond;

  EXPECT_EQ(orig.seconds() + 1, var.seconds());
  EXPECT_EQ(orig.microSeconds() - 1, var.microSeconds());
}

TEST(Date, SecondSubtraction) {
  const Date second(1, 0);
  const Date orig(Time::Instance().now());
  Date var(orig);

  var -= 1;

  EXPECT_EQ(orig.seconds() - 1, var.seconds());
  EXPECT_EQ(orig.microSeconds(), var.microSeconds());

  var = orig;
  var -= second;

  EXPECT_EQ(orig.seconds() - 1, var.seconds());
  EXPECT_EQ(orig.microSeconds(), var.microSeconds());

  var = orig - second;

  EXPECT_EQ(orig.seconds() - 1, var.seconds());
  EXPECT_EQ(orig.microSeconds(), var.microSeconds());
}

TEST(Date, MicroSecondSubtraction) {
  const Date microsecond(0, 1);
  const Date orig(Time::Instance().now());
  Date var(orig);

  var = orig;
  var -= microsecond;

  EXPECT_EQ(orig.seconds(), var.seconds());
  EXPECT_EQ(orig.microSeconds() - 1, var.microSeconds());

  var = orig - microsecond;

  EXPECT_EQ(orig.seconds(), var.seconds());
  EXPECT_EQ(orig.microSeconds() - 1, var.microSeconds());
}

TEST(Date, MicroSecondSubtractionWrap) {
  const Date nearSecond(0, 999999);
  const Date orig(Time::Instance().now());
  Date var(orig);

  var = orig;
  var -= nearSecond;

  EXPECT_EQ(orig.seconds() - 1, var.seconds());
  EXPECT_EQ(orig.microSeconds() + 1, var.microSeconds());

  var = orig - nearSecond;

  EXPECT_EQ(orig.seconds() - 1, var.seconds());
  EXPECT_EQ(orig.microSeconds() + 1, var.microSeconds());
}

TEST(Date, SecondComparisons) {
  const Date orig(Time::Instance().now());
  Date var(orig);

  EXPECT_TRUE(orig == var);
  EXPECT_FALSE(orig < var);
  EXPECT_FALSE(orig > var);
  EXPECT_TRUE(orig <= var);
  EXPECT_TRUE(orig >= var);

  var += 1;

  EXPECT_FALSE(orig == var);
  EXPECT_TRUE(orig < var);
  EXPECT_FALSE(orig > var);
  EXPECT_TRUE(orig <= var);
  EXPECT_FALSE(orig >= var);

  var = orig - 1;

  EXPECT_FALSE(orig == var);
  EXPECT_FALSE(orig < var);
  EXPECT_TRUE(orig > var);
  EXPECT_FALSE(orig <= var);
  EXPECT_TRUE(orig >= var);
}

TEST(Date, MicroSecondWrapComparisons) {
  const Date nearSecond(0, 999999);
  const Date orig(Time::Instance().now());
  Date var(orig);

  EXPECT_TRUE(orig == var);
  EXPECT_FALSE(orig < var);
  EXPECT_FALSE(orig > var);
  EXPECT_TRUE(orig <= var);
  EXPECT_TRUE(orig >= var);

  var += nearSecond;

  EXPECT_FALSE(orig == var);
  EXPECT_TRUE(orig < var);
  EXPECT_FALSE(orig > var);
  EXPECT_TRUE(orig <= var);
  EXPECT_FALSE(orig >= var);

  var = orig - nearSecond;

  EXPECT_FALSE(orig == var);
  EXPECT_FALSE(orig < var);
  EXPECT_TRUE(orig > var);
  EXPECT_FALSE(orig <= var);
  EXPECT_TRUE(orig >= var);
}

TEST(Date, MicroSecondComparisons) {
  const Date usec(0, 1);
  const Date orig(Time::Instance().now());
  Date var(orig);

  EXPECT_TRUE(orig == var);
  EXPECT_FALSE(orig < var);
  EXPECT_FALSE(orig > var);
  EXPECT_TRUE(orig <= var);
  EXPECT_TRUE(orig >= var);

  var += usec;

  EXPECT_FALSE(orig == var);
  EXPECT_TRUE(orig < var);
  EXPECT_FALSE(orig > var);
  EXPECT_TRUE(orig <= var);
  EXPECT_FALSE(orig >= var);

  var = orig - usec;

  EXPECT_FALSE(orig == var);
  EXPECT_FALSE(orig < var);
  EXPECT_TRUE(orig > var);
  EXPECT_FALSE(orig <= var);
  EXPECT_TRUE(orig >= var);
}
