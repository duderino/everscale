#ifndef ESB_RAND_H
#include <ESBRand.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(Rand, Int32MinMax) {
  Rand rand(42);

  for (int i = 0; i < 42; ++i) {
    Int32 random = rand.generate(ESB_INT32_MIN, ESB_INT32_MAX);
    ASSERT_NE(random, 0);  // seed guarantees we won't hit this, but otherwise it's possible.
    ASSERT_GE(random, ESB_INT32_MIN);
    ASSERT_LE(random, ESB_INT32_MAX);
  }
}

TEST(Rand, Int32NarrowRange) {
  Rand rand(42);
  const Int32 min = -42;
  const Int32 max = 42;

  for (int i = 0; i < 42; ++i) {
    Int32 random = rand.generate(min, max);
    ASSERT_GE(random, min);
    ASSERT_LE(random, max);
  }
}

TEST(Rand, UInt32MinMax) {
  Rand rand(42);

  for (int i = 0; i < 42; ++i) {
    UInt32 random = rand.generate(ESB_UINT32_MIN, ESB_UINT32_MAX);
    ASSERT_NE(random, 0);  // seed guarantees we won't hit this, but otherwise it's possible.
    ASSERT_GE(random, ESB_UINT32_MIN);
    ASSERT_LE(random, ESB_UINT32_MAX);
  }
}

TEST(Rand, UInt32NarrowRange) {
  Rand rand(42);
  const UInt32 min = 0;
  const UInt32 max = 42;

  for (int i = 0; i < 42; ++i) {
    UInt32 random = rand.generate(min, max);
    ASSERT_GE(random, min);
    ASSERT_LE(random, max);
  }
}
