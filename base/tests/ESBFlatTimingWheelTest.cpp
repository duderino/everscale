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
  UInt32 _calls;

  ESB_DISABLE_AUTO_COPY(TestCleanupHandler);
};

class TestTimer : public Timer {
 public:
  TestTimer(TestCleanupHandler *cleanupHandler = NULL) : _cleanupHandler(cleanupHandler) {}
  virtual ~TestTimer() {}

  virtual CleanupHandler *cleanupHandler() { return _cleanupHandler; }

  void setCleanupHandler(CleanupHandler *handler) { _cleanupHandler = handler; }

 private:
  CleanupHandler *_cleanupHandler;

  ESB_DISABLE_AUTO_COPY(TestTimer);
};

class TimingWheelTest : public ::testing::Test {
 public:
  // starting microseconds at 0 keeps the test deterministic
  TimingWheelTest() : _timeSource(Date(SystemTimeSource::Instance().now().seconds(), 0)) {}

  virtual void SetUp() { Time::Instance().setTimeSource(_timeSource); }

  virtual void TearDown() { Time::Instance().setTimeSource(SystemTimeSource::Instance()); }

 protected:
  FakeTimeSource _timeSource;

 private:
  // Disabled
  TimingWheelTest(const TimingWheelTest &);
  TimingWheelTest &operator=(const TimingWheelTest &);
};

TEST_F(TimingWheelTest, Underflow) {
  TestTimer timer;
  UInt32 ticks = 10;
  UInt32 tickMilliSeconds = 1;
  FlatTimingWheel timingWheel(ticks, tickMilliSeconds);

  // Cannot schedule in the current tick
  Error error = timingWheel.insert(&timer, 0);
  EXPECT_EQ(ESB_UNDERFLOW, error);
}

TEST_F(TimingWheelTest, Overflow) {
  TestTimer timer;
  UInt32 ticks = 10;
  UInt32 tickMilliSeconds = 1;
  FlatTimingWheel timingWheel(ticks, tickMilliSeconds);

  // Cannot schedule past the timing wheel range
  Error error = timingWheel.insert(&timer, ticks * tickMilliSeconds);
  EXPECT_EQ(ESB_OVERFLOW, error);
}

TEST_F(TimingWheelTest, OnePerTick) {
  const UInt32 ticks = 5;
  TestTimer timers[ticks];
  const UInt32 tickMilliSeconds = 1;
  FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds);

  // Schedule

  for (UInt32 i = 0; i < ticks; ++i) {
    UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    Error error = timingWheel.insert(&timers[i], expirationMilliSeconds);
    EXPECT_EQ(ESB_SUCCESS, error);
  }

  // Initially nothing expires
  Timer *timer = timingWheel.nextExpired();
  EXPECT_EQ(NULL, timer);

  // Then exactly one timer should expire every tick/millisecond

  for (UInt32 i = 0; i < ticks; ++i) {
    _timeSource.addMilliSeconds(tickMilliSeconds);
    timer = timingWheel.nextExpired();
    EXPECT_TRUE(timer);
    timer = timingWheel.nextExpired();
    EXPECT_EQ(NULL, timer);
  }

  // Then nothing

  _timeSource.addMilliSeconds(tickMilliSeconds);
  timer = timingWheel.nextExpired();
  EXPECT_EQ(NULL, timer);
}

TEST_F(TimingWheelTest, ManyPerTick) {
  const UInt32 ticks = 5;
  const UInt32 timersPerTick = 3;
  TestTimer timers[ticks][timersPerTick];
  const UInt32 tickMilliSeconds = 1;
  FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds);

  // Schedule

  for (UInt32 i = 0U; i < ticks; ++i) {
    const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds);
      EXPECT_EQ(ESB_SUCCESS, error);
    }
  }

  // Initially nothing expires
  Timer *timer = timingWheel.nextExpired();
  EXPECT_EQ(NULL, timer);

  // Then exactly 3 timers should expire every tick/millisecond

  for (UInt32 i = 0U; i < ticks; ++i) {
    _timeSource.addMilliSeconds(tickMilliSeconds);
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      timer = timingWheel.nextExpired();
      EXPECT_TRUE(timer);
    }
    timer = timingWheel.nextExpired();
    EXPECT_EQ(NULL, timer);
  }

  // Then nothing

  _timeSource.addMilliSeconds(tickMilliSeconds);
  timer = timingWheel.nextExpired();
  EXPECT_EQ(NULL, timer);
}

TEST_F(TimingWheelTest, CompleteTick) {
  const UInt32 ticks = 5;
  const UInt32 timersPerTick = 3;
  TestTimer timers[ticks][timersPerTick];
  const UInt32 tickMilliSeconds = 3;
  FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds);

  // Schedule

  for (UInt32 i = 0U; i < ticks; ++i) {
    const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds);
      EXPECT_EQ(ESB_SUCCESS, error);
    }
  }

  // Initially nothing expires
  Timer *timer = timingWheel.nextExpired();
  EXPECT_EQ(NULL, timer);

  // Then exactly 3 timers should expire every tick

  for (UInt32 i = 0U; i < ticks; ++i) {
    // Nothing should expire before we have a complete tick
    for (UInt32 j = 0U; j < tickMilliSeconds - 1; ++j) {
      _timeSource.addMilliSeconds(1);
      timer = timingWheel.nextExpired();
      EXPECT_EQ(NULL, timer);
    }

    // Now all three expire since we have a complete tick
    _timeSource.addMilliSeconds(1);
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      timer = timingWheel.nextExpired();
      EXPECT_TRUE(timer);
    }

    timer = timingWheel.nextExpired();
    EXPECT_EQ(NULL, timer);
  }

  // Then nothing

  _timeSource.addMilliSeconds(tickMilliSeconds);
  timer = timingWheel.nextExpired();
  EXPECT_EQ(NULL, timer);
}

TEST_F(TimingWheelTest, CatchupAfterNeglect) {
  const UInt32 ticks = 6;
  const UInt32 timersPerTick = 3;
  TestTimer timers[ticks][timersPerTick];
  const UInt32 tickMilliSeconds = 3;
  FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds);

  // Schedule

  for (UInt32 i = 0U; i < ticks; ++i) {
    const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds);
      EXPECT_EQ(ESB_SUCCESS, error);
    }
  }

  // Initially nothing expires
  Timer *timer = timingWheel.nextExpired();
  EXPECT_EQ(NULL, timer);

  // Wait half the ticks, half the timers should expire
  _timeSource.addMilliSeconds(tickMilliSeconds * ticks / 2);

  for (UInt32 i = 0U; i < ticks * timersPerTick / 2; ++i) {
    timer = timingWheel.nextExpired();
    EXPECT_TRUE(timer);
  }

  timer = timingWheel.nextExpired();
  EXPECT_EQ(NULL, timer);

  // Wait a ton of time
  _timeSource.addMilliSeconds(tickMilliSeconds * 100);

  {
    // Timing wheel window is full until we drain the expired timers
    TestTimer testTimer;
    Error error = timingWheel.insert(&testTimer, tickMilliSeconds);
    EXPECT_EQ(ESB_OVERFLOW, error);
  }

  // The rest of the timers expired long ago

  for (UInt32 i = 0U; i < ticks * timersPerTick / 2; ++i) {
    timer = timingWheel.nextExpired();
    EXPECT_TRUE(timer);
  }

  timer = timingWheel.nextExpired();
  EXPECT_EQ(NULL, timer);

  {
    // Now that we've drained the expired timers, we can insert new ones even though fake time has not advanced.
    TestTimer testTimer;
    Error error = timingWheel.insert(&testTimer, tickMilliSeconds);
    EXPECT_EQ(ESB_SUCCESS, error);
  }
}

TEST_F(TimingWheelTest, Cancellation) {
  const UInt32 ticks = 5;
  const UInt32 timersPerTick = 3;
  TestTimer timers[ticks][timersPerTick];
  const UInt32 tickMilliSeconds = 1;
  FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds);

  // Schedule

  for (UInt32 i = 0U; i < ticks; ++i) {
    const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds);
      EXPECT_EQ(ESB_SUCCESS, error);
    }
  }

  // Initially nothing expires
  Timer *timer = timingWheel.nextExpired();
  EXPECT_EQ(NULL, timer);

  // Cancel 1 timer per tick

  for (UInt32 i = 0U; i < ticks; ++i) {
    Error error = timingWheel.remove(&timers[i][1]);
    EXPECT_EQ(ESB_SUCCESS, error);
  }

  // Then exactly 2 timers should expire every tick/millisecond

  for (UInt32 i = 0U; i < ticks; ++i) {
    _timeSource.addMilliSeconds(tickMilliSeconds);
    for (UInt32 j = 0U; j < timersPerTick - 1; ++j) {
      timer = timingWheel.nextExpired();
      EXPECT_TRUE(timer);
    }
    timer = timingWheel.nextExpired();
    EXPECT_EQ(NULL, timer);
  }

  // Then nothing

  _timeSource.addMilliSeconds(tickMilliSeconds);
  timer = timingWheel.nextExpired();
  EXPECT_EQ(NULL, timer);
}

TEST_F(TimingWheelTest, Clear) {
  const UInt32 ticks = 5;
  const UInt32 timersPerTick = 3;
  TestTimer timers[ticks][timersPerTick];
  const UInt32 tickMilliSeconds = 1;
  FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds);

  // Schedule

  for (UInt32 i = 0U; i < ticks; ++i) {
    const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds);
      EXPECT_EQ(ESB_SUCCESS, error);
    }
  }

  timingWheel.clear();

  // Then nothing should expire every tick/millisecond

  for (UInt32 i = 0U; i < ticks; ++i) {
    _timeSource.addMilliSeconds(tickMilliSeconds);
    Timer *timer = timingWheel.nextExpired();
    EXPECT_EQ(NULL, timer);
  }
}

TEST_F(TimingWheelTest, CleanupHandlerClear) {
  TestCleanupHandler cleanupHandler;
  const UInt32 ticks = 5;
  const UInt32 timersPerTick = 3;
  TestTimer timers[ticks][timersPerTick];
  const UInt32 tickMilliSeconds = 1;
  FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds);

  // Schedule w. cleanup handlers

  for (UInt32 i = 0U; i < ticks; ++i) {
    const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      timers[i][j].setCleanupHandler(&cleanupHandler);
      Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds);
      EXPECT_EQ(ESB_SUCCESS, error);
    }
  }

  // clear with timers, should call cleanup handler
  timingWheel.clear();

  // cleanup handler should have been called 15 times
  EXPECT_EQ(ticks * timersPerTick, cleanupHandler.calls());
}

TEST_F(TimingWheelTest, CleanupHandlerDestructor) {
  TestCleanupHandler cleanupHandler;
  const UInt32 ticks = 5;
  const UInt32 timersPerTick = 3;
  TestTimer timers[ticks][timersPerTick];
  const UInt32 tickMilliSeconds = 1;

  {
    FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds);

    // Schedule w. cleanup handlers

    for (UInt32 i = 0U; i < ticks; ++i) {
      const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
      for (UInt32 j = 0U; j < timersPerTick; ++j) {
        timers[i][j].setCleanupHandler(&cleanupHandler);
        Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds);
        EXPECT_EQ(ESB_SUCCESS, error);
      }
    }
  }

  // cleanup handler should have been called 15 times
  EXPECT_EQ(ticks * timersPerTick, cleanupHandler.calls());
}
