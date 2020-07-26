#ifndef ESB_FLAT_TIMING_WHEEL_H
#include <ESBFlatTimingWheel.h>
#endif

#ifndef ESB_TIME_H
#include <ESBTime.h>
#endif

#ifndef ESB_SYSTEM_TIME_SOURCE_H
#include <ESBSystemTimeSource.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

class TestCleanupHandler : public CleanupHandler {
 public:
  TestCleanupHandler() : _calls(0U) {}
  virtual ~TestCleanupHandler() {}

  inline UInt32 calls() const { return _calls; }

  virtual void destroy(Object *object) { ++_calls; }

 private:
  // Disabled
  TestCleanupHandler(const TestCleanupHandler &);
  TestCleanupHandler &operator=(const TestCleanupHandler &);

  UInt32 _calls;
};

class TestDelayedCommand : public DelayedCommand {
 public:
  TestDelayedCommand(TestCleanupHandler *cleanupHandler = NULL) : _calls(0U), _cleanupHandler(cleanupHandler) {}
  virtual ~TestDelayedCommand() {}

  inline UInt32 calls() const { return _calls; }

  virtual const char *name() const { return "TestDelayedCommand"; }

  virtual bool run(SharedInt *isRunning) {
    ++_calls;
    return _cleanupHandler ? true : false;
  }

  virtual CleanupHandler *cleanupHandler() { return _cleanupHandler; }

  void setCleanupHandler(CleanupHandler *handler) { _cleanupHandler = handler; }

 private:
  // Disabled
  TestDelayedCommand(const TestDelayedCommand &);
  TestDelayedCommand &operator=(const TestDelayedCommand &);

  UInt32 _calls;
  CleanupHandler *_cleanupHandler;
};

class TimingWheelTest : public ::testing::Test {
 public:
  TimingWheelTest() : _timeSource(SystemTimeSource::Instance().now()) {}

  virtual void SetUp() { Time::Instance().setTimeSource(_timeSource); }

  virtual void TearDown() { Time::Instance().setTimeSource(SystemTimeSource::Instance()); }

 protected:
  FakeTimeSource _timeSource;

 private:
  // Disabled
  TimingWheelTest(const TimingWheelTest &);
  TimingWheelTest &operator=(const TimingWheelTest &);
};

static SharedInt IsRunning(true);

static UInt32 totalCalls(TestDelayedCommand *commands, int size) {
  UInt32 totalCalls = 0U;
  for (int i = 0; i < size; ++i) {
    totalCalls += commands[i].calls();
  }
  return totalCalls;
}

TEST_F(TimingWheelTest, TimelyExecution) {
  TestDelayedCommand commands[5];
  UInt32 numCommands = sizeof(commands) / sizeof(TestDelayedCommand);
  UInt32 ticks = 10;
  UInt32 tickMilliSeconds = 1;
  FlatTimingWheel timingWheel(ticks, tickMilliSeconds);

  // Schedule

  for (UInt32 i = 0; i < numCommands; ++i) {
    Error error = timingWheel.insert(&commands[i], i);
    EXPECT_EQ(ESB_SUCCESS, error);
  }

  // Exec

  for (UInt32 i = 0; i <= numCommands; ++i) {
    Error error = timingWheel.run(&IsRunning);
    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(i, totalCalls(commands, numCommands));
    _timeSource.addMilliSeconds(1);
  }
}

TEST_F(TimingWheelTest, CatchupExecution) {
  TestDelayedCommand commands[5];
  UInt32 numCommands = sizeof(commands) / sizeof(TestDelayedCommand);
  UInt32 ticks = 10;
  UInt32 tickMilliSeconds = 1;
  FlatTimingWheel timingWheel(ticks, tickMilliSeconds);

  // Schedule

  for (UInt32 i = 0; i < numCommands; ++i) {
    Error error = timingWheel.insert(&commands[i], i);
    EXPECT_EQ(ESB_SUCCESS, error);
  }

  // Exec

  Error error = timingWheel.run(&IsRunning);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, totalCalls(commands, numCommands));

  _timeSource.addMilliSeconds(2);
  error = timingWheel.run(&IsRunning);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(2, totalCalls(commands, numCommands));

  _timeSource.addMilliSeconds(6);
  error = timingWheel.run(&IsRunning);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(numCommands, totalCalls(commands, numCommands));
}

TEST_F(TimingWheelTest, Overflow) {
  TestDelayedCommand commands[5];
  UInt32 numCommands = sizeof(commands) / sizeof(TestDelayedCommand);
  UInt32 ticks = 3;
  UInt32 tickMilliSeconds = 1;
  FlatTimingWheel timingWheel(ticks, tickMilliSeconds);

  // Schedule

  for (UInt32 i = 0; i < ticks; ++i) {
    Error error = timingWheel.insert(&commands[i], i);
    EXPECT_EQ(ESB_SUCCESS, error);
  }

  for (UInt32 i = ticks; i < numCommands; ++i) {
    Error error = timingWheel.insert(&commands[i], i);
    EXPECT_EQ(ESB_INVALID_ARGUMENT, error);
  }
}

TEST_F(TimingWheelTest, OverflowAndCap) {
  TestDelayedCommand commands[5];
  UInt32 numCommands = sizeof(commands) / sizeof(TestDelayedCommand);
  UInt32 ticks = 3;
  UInt32 tickMilliSeconds = 1;
  FlatTimingWheel timingWheel(ticks, tickMilliSeconds);

  // Schedule

  for (UInt32 i = 0; i < numCommands; ++i) {
    Error error = timingWheel.insert(&commands[i], i, true);
    EXPECT_EQ(ESB_SUCCESS, error);
  }

  // Exec

  for (UInt32 i = 0; i < ticks; ++i) {
    Error error = timingWheel.run(&IsRunning);
    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(i, totalCalls(commands, numCommands));
    _timeSource.addMilliSeconds(1);
  }

  // All overflows should exec on the last tick

  _timeSource.addMilliSeconds(1);
  Error error = timingWheel.run(&IsRunning);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(numCommands, totalCalls(commands, numCommands));
}

TEST_F(TimingWheelTest, Cancellation) {
  TestDelayedCommand commands[5];
  UInt32 numCommands = sizeof(commands) / sizeof(TestDelayedCommand);
  UInt32 ticks = 10;
  UInt32 tickMilliSeconds = 1;
  FlatTimingWheel timingWheel(ticks, tickMilliSeconds);

  // Schedule

  for (UInt32 i = 0; i < numCommands; ++i) {
    Error error = timingWheel.insert(&commands[i], i);
    EXPECT_EQ(ESB_SUCCESS, error);
  }

  // Cancel

  timingWheel.remove(&commands[1]);
  timingWheel.remove(&commands[3]);

  // Exec

  _timeSource.addMilliSeconds(numCommands);
  Error error = timingWheel.run(&IsRunning);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(numCommands - 2, totalCalls(commands, numCommands));
}

TEST_F(TimingWheelTest, CleanupHandler) {
  TestCleanupHandler cleanupHandler;
  TestDelayedCommand commands[5];
  UInt32 numCommands = sizeof(commands) / sizeof(TestDelayedCommand);
  UInt32 ticks = 10;
  UInt32 tickMilliSeconds = 1;
  FlatTimingWheel timingWheel(ticks, tickMilliSeconds);

  // Schedule

  for (UInt32 i = 0; i < numCommands; ++i) {
    commands[i].setCleanupHandler(&cleanupHandler);
    Error error = timingWheel.insert(&commands[i], i);
    EXPECT_EQ(ESB_SUCCESS, error);
  }

  // Exec

  for (UInt32 i = 0; i <= numCommands; ++i) {
    Error error = timingWheel.run(&IsRunning);
    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(i, totalCalls(commands, numCommands));
    EXPECT_EQ(i, cleanupHandler.calls());
    _timeSource.addMilliSeconds(1);
  }
}