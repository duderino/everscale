#ifndef ESB_SIGNAL_HANDLER_H
#include <ESBSignalHandler.h>
#endif

#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>

using namespace ESB;

static SimpleFileLogger TestLogger(stdout, Logger::Warning);

class SignalHandlerTest : public ::testing::Test {
 public:
  static void SetUpTestSuite() {
    Logger::SetInstance(&TestLogger);
    EXPECT_EQ(ESB_SUCCESS, SignalHandler::Instance().initialize());
  }

  static void TearDownTestSuite() { Logger::SetInstance(NULL); }
};

TEST_F(SignalHandlerTest, DivideByZero) {
  volatile int denominator = 0;  // volatile - prevent optimizer from optimizing statement away
  int result = 0;
  EXPECT_EXIT(result = 42 / denominator, ::testing::KilledBySignal(SIGFPE), ".*");
  printf("%d\n", result);  // prevent optimizer from optimizing statement away
}

TEST_F(SignalHandlerTest, DereferenceNullPointer) {
  char *pointer = NULL;
#ifdef NDEBUG
  ASSERT_EXIT(*pointer = 'a', ::testing::KilledBySignal(SIGILL), ".*");
#else
  ASSERT_EXIT(*pointer = 'a', ::testing::KilledBySignal(SIGSEGV), ".*");
#endif
}

TEST_F(SignalHandlerTest, Assert) {
#ifndef NDEBUG
  ASSERT_EXIT(assert(false), ::testing::KilledBySignal(SIGABRT), ".*");
#endif
}

TEST_F(SignalHandlerTest, CatchSigTerm) {
  EXPECT_EQ(true, SignalHandler::Instance().running());
  raise(SIGTERM);
  EXPECT_EQ(false, SignalHandler::Instance().running());
}
