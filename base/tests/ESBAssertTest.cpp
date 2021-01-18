#ifndef ESB_ASSERT_H
#include <ESBAssert.h>
#endif

#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
#endif

#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>

using namespace ESB;

static SimpleFileLogger TestLogger(stdout, Logger::Warning);

class AssertTest : public ::testing::Test {
 public:
  static void SetUpTestSuite() { Logger::SetInstance(&TestLogger); }

  static void TearDownTestSuite() { Logger::SetInstance(NULL); }
};

TEST_F(AssertTest, FailedAssertion) { EXPECT_DEBUG_DEATH(ESB_ASSERT(1 == 2), "assert"); }

TEST_F(AssertTest, PassedAssertion) { ESB_ASSERT(1 == 1); }
