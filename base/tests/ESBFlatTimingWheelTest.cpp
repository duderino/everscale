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

class CleanupTimer : public Timer {
 public:
  CleanupTimer(TestCleanupHandler *cleanupHandler = NULL) : _cleanupHandler(cleanupHandler) {}
  virtual ~CleanupTimer() {}

  virtual CleanupHandler *cleanupHandler() { return _cleanupHandler; }

  void setCleanupHandler(CleanupHandler *handler) { _cleanupHandler = handler; }

 private:
  CleanupHandler *_cleanupHandler;

  ESB_DISABLE_AUTO_COPY(CleanupTimer);
};

TEST(TimingWheelTest, Underflow) {
  Date now(Time::Instance().now().seconds(), 0);
  Timer timer;
  UInt32 ticks = 10;
  UInt32 tickMilliSeconds = 1;
  FlatTimingWheel timingWheel(ticks, tickMilliSeconds, now);

  // Cannot schedule in the current tick
  Error error = timingWheel.insert(&timer, 0, now);
  EXPECT_EQ(ESB_UNDERFLOW, error);
}

TEST(TimingWheelTest, Overflow) {
  Date now(Time::Instance().now().seconds(), 0);
  Timer timer;
  UInt32 ticks = 10;
  UInt32 tickMilliSeconds = 1;
  FlatTimingWheel timingWheel(ticks, tickMilliSeconds, now);

  // Cannot schedule past the timing wheel range
  Error error = timingWheel.insert(&timer, ticks * tickMilliSeconds, now);
  EXPECT_EQ(ESB_OVERFLOW, error);
}

TEST(TimingWheelTest, OnePerTick) {
  const UInt32 ticks = 5;
  const UInt32 tickMilliSeconds = 1;
  Date now(Time::Instance().now().seconds(), 0);
  const Date timeIncrement(0, tickMilliSeconds * 1000);
  CleanupTimer timers[ticks];
  FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds, now);

  // Schedule

  for (UInt32 i = 0; i < ticks; ++i) {
    EXPECT_FALSE(timers[i].inTimingWheel());
    UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    Error error = timingWheel.insert(&timers[i], expirationMilliSeconds, now);
    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_TRUE(timers[i].inTimingWheel());
  }

  // Initially nothing expires
  Timer *timer = timingWheel.nextExpired(now);
  EXPECT_EQ(NULL, timer);

  // Then exactly one timer should expire every tick/millisecond

  for (UInt32 i = 0; i < ticks; ++i) {
    now += timeIncrement;
    timer = timingWheel.nextExpired(now);
    EXPECT_TRUE(timer);
    EXPECT_FALSE(timer->inTimingWheel());
    timer = timingWheel.nextExpired(now);
    EXPECT_EQ(NULL, timer);
  }

  // Then nothing

  now += timeIncrement;
  timer = timingWheel.nextExpired(now);
  EXPECT_EQ(NULL, timer);
}

TEST(TimingWheelTest, ManyPerTick) {
  const UInt32 ticks = 5;
  const UInt32 timersPerTick = 3;
  const UInt32 tickMilliSeconds = 1;
  Date now(Time::Instance().now().seconds(), 0);
  const Date timeIncrement(0, tickMilliSeconds * 1000);
  Timer timers[ticks][timersPerTick];
  FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds, now);

  // Schedule

  for (UInt32 i = 0U; i < ticks; ++i) {
    const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      EXPECT_FALSE(timers[i][j].inTimingWheel());
      Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds, now);
      EXPECT_EQ(ESB_SUCCESS, error);
      EXPECT_TRUE(timers[i][j].inTimingWheel());
    }
  }

  // Initially nothing expires
  Timer *timer = timingWheel.nextExpired(now);
  EXPECT_EQ(NULL, timer);

  // Then exactly 3 timers should expire every tick/millisecond

  for (UInt32 i = 0U; i < ticks; ++i) {
    now += timeIncrement;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      timer = timingWheel.nextExpired(now);
      EXPECT_TRUE(timer);
      EXPECT_FALSE(timer->inTimingWheel());
    }
    timer = timingWheel.nextExpired(now);
    EXPECT_EQ(NULL, timer);
  }

  // Then nothing

  now += timeIncrement;
  timer = timingWheel.nextExpired(now);
  EXPECT_EQ(NULL, timer);
}

TEST(TimingWheelTest, CompleteTick) {
  const UInt32 ticks = 5;
  const UInt32 timersPerTick = 3;
  const UInt32 tickMilliSeconds = 1;
  Date now(Time::Instance().now().seconds(), 0);
  const Date timeIncrement(0, 1000);
  Timer timers[ticks][timersPerTick];
  FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds, now);

  // Schedule

  for (UInt32 i = 0U; i < ticks; ++i) {
    const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      EXPECT_FALSE(timers[i][j].inTimingWheel());
      Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds, now);
      EXPECT_EQ(ESB_SUCCESS, error);
      EXPECT_TRUE(timers[i][j].inTimingWheel());
    }
  }

  // Initially nothing expires
  Timer *timer = timingWheel.nextExpired(now);
  EXPECT_EQ(NULL, timer);

  // Then exactly 3 timers should expire every tick

  for (UInt32 i = 0U; i < ticks; ++i) {
    // Nothing should expire before we have a complete tick
    for (UInt32 j = 0U; j < tickMilliSeconds - 1; ++j) {
      now += timeIncrement;
      timer = timingWheel.nextExpired(now);
      EXPECT_EQ(NULL, timer);
    }

    // Now all three expire since we have a complete tick
    now += timeIncrement;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      timer = timingWheel.nextExpired(now);
      EXPECT_TRUE(timer);
      EXPECT_FALSE(timer->inTimingWheel());
    }

    timer = timingWheel.nextExpired(now);
    EXPECT_EQ(NULL, timer);
  }

  // Then nothing

  now += timeIncrement + timeIncrement + timeIncrement;
  timer = timingWheel.nextExpired(now);
  EXPECT_EQ(NULL, timer);
}

TEST(TimingWheelTest, CatchupAfterNeglect) {
  const UInt32 ticks = 6;
  const UInt32 timersPerTick = 3;
  const UInt32 tickMilliSeconds = 3;
  Date now(Time::Instance().now().seconds(), 0);
  const Date timeIncrement(0, tickMilliSeconds * ticks * 1000 / 2);
  Timer timers[ticks][timersPerTick];
  FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds, now);

  // Schedule

  for (UInt32 i = 0U; i < ticks; ++i) {
    const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      EXPECT_FALSE(timers[i][j].inTimingWheel());
      Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds, now);
      EXPECT_EQ(ESB_SUCCESS, error);
      EXPECT_TRUE(timers[i][j].inTimingWheel());
    }
  }

  // Initially nothing expires
  Timer *timer = timingWheel.nextExpired(now);
  EXPECT_EQ(NULL, timer);

  // Wait half the ticks, half the timers should expire
  now += timeIncrement;

  for (UInt32 i = 0U; i < ticks * timersPerTick / 2; ++i) {
    timer = timingWheel.nextExpired(now);
    EXPECT_TRUE(timer);
    EXPECT_FALSE(timer->inTimingWheel());
  }

  timer = timingWheel.nextExpired(now);
  EXPECT_EQ(NULL, timer);

  // Negleck for a long time (ten seconds)
  now += 10;

  {
    // Timing wheel window is full until we drain the expired timers
    CleanupTimer testTimer;
    Error error = timingWheel.insert(&testTimer, tickMilliSeconds, now);
    EXPECT_EQ(ESB_OVERFLOW, error);
    EXPECT_FALSE(testTimer.inTimingWheel());
  }

  // The rest of the timers expired long ago

  for (UInt32 i = 0U; i < ticks * timersPerTick / 2; ++i) {
    timer = timingWheel.nextExpired(now);
    EXPECT_TRUE(timer);
    EXPECT_FALSE(timer->inTimingWheel());
  }

  timer = timingWheel.nextExpired(now);
  EXPECT_EQ(NULL, timer);

  {
    // Now that we've drained the expired timers, we can insert new ones even though fake time has not advanced.
    CleanupTimer testTimer;
    Error error = timingWheel.insert(&testTimer, tickMilliSeconds, now);
    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_TRUE(testTimer.inTimingWheel());
  }
}

TEST(TimingWheelTest, Cancellation) {
  const UInt32 ticks = 5;
  const UInt32 timersPerTick = 3;
  const UInt32 tickMilliSeconds = 1;
  Date now(Time::Instance().now().seconds(), 0);
  const Date timeIncrement(0, tickMilliSeconds * 1000);
  Timer timers[ticks][timersPerTick];
  FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds, now);

  // Schedule

  for (UInt32 i = 0U; i < ticks; ++i) {
    const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      EXPECT_FALSE(timers[i][j].inTimingWheel());
      Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds, now);
      EXPECT_EQ(ESB_SUCCESS, error);
      EXPECT_TRUE(timers[i][j].inTimingWheel());
    }
  }

  // Initially nothing expires
  Timer *timer = timingWheel.nextExpired(now);
  EXPECT_EQ(NULL, timer);

  // Cancel 1 timer per tick

  for (UInt32 i = 0U; i < ticks; ++i) {
    Error error = timingWheel.remove(&timers[i][1]);
    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_TRUE(timers[i][0].inTimingWheel());
    EXPECT_FALSE(timers[i][1].inTimingWheel());
    EXPECT_TRUE(timers[i][2].inTimingWheel());
  }

  // Then exactly 2 timers should expire every tick/millisecond

  for (UInt32 i = 0U; i < ticks; ++i) {
    now += timeIncrement;
    for (UInt32 j = 0U; j < timersPerTick - 1; ++j) {
      timer = timingWheel.nextExpired(now);
      EXPECT_TRUE(timer);
      EXPECT_FALSE(timer->inTimingWheel());
    }
    timer = timingWheel.nextExpired(now);
    EXPECT_EQ(NULL, timer);
  }

  // Then nothing

  now += timeIncrement;
  timer = timingWheel.nextExpired(now);
  EXPECT_EQ(NULL, timer);
}

TEST(TimingWheelTest, Clear) {
  const UInt32 ticks = 5;
  const UInt32 timersPerTick = 3;
  const UInt32 tickMilliSeconds = 1;
  Date now(Time::Instance().now().seconds(), 0);
  const Date timeIncrement(0, tickMilliSeconds * 1000);
  Timer timers[ticks][timersPerTick];
  FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds, now);

  // Schedule

  for (UInt32 i = 0U; i < ticks; ++i) {
    const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      EXPECT_FALSE(timers[i][j].inTimingWheel());
      Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds, now);
      EXPECT_EQ(ESB_SUCCESS, error);
      EXPECT_TRUE(timers[i][j].inTimingWheel());
    }
  }

  timingWheel.clear();

  for (UInt32 i = 0U; i < ticks; ++i) {
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      EXPECT_FALSE(timers[i][j].inTimingWheel());
    }
  }

  // Then nothing should expire every tick/millisecond

  for (UInt32 i = 0U; i < ticks; ++i) {
    now += timeIncrement;
    Timer *timer = timingWheel.nextExpired(now);
    EXPECT_EQ(NULL, timer);
  }
}

TEST(TimingWheelTest, CleanupHandlerClear) {
  TestCleanupHandler cleanupHandler;
  const UInt32 ticks = 5;
  const UInt32 timersPerTick = 3;
  const UInt32 tickMilliSeconds = 1;
  Date now(Time::Instance().now().seconds(), 0);
  const Date timeIncrement(0, tickMilliSeconds * 1000);
  CleanupTimer timers[ticks][timersPerTick];
  FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds, now);

  // Schedule w. cleanup handlers

  for (UInt32 i = 0U; i < ticks; ++i) {
    const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      EXPECT_FALSE(timers[i][j].inTimingWheel());
      timers[i][j].setCleanupHandler(&cleanupHandler);
      Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds, now);
      EXPECT_EQ(ESB_SUCCESS, error);
      EXPECT_TRUE(timers[i][j].inTimingWheel());
    }
  }

  // clear with timers, should call cleanup handler
  timingWheel.clear();

  // cleanup handler should have been called 15 times
  EXPECT_EQ(ticks * timersPerTick, cleanupHandler.calls());
}

TEST(TimingWheelTest, CleanupHandlerDestructor) {
  TestCleanupHandler cleanupHandler;
  const UInt32 ticks = 5;
  const UInt32 timersPerTick = 3;
  const UInt32 tickMilliSeconds = 1;
  Date now(Time::Instance().now().seconds(), 0);
  const Date timeIncrement(0, tickMilliSeconds * 1000);
  CleanupTimer timers[ticks][timersPerTick];

  {
    FlatTimingWheel timingWheel(ticks + 1, tickMilliSeconds, now);

    // Schedule w. cleanup handlers

    for (UInt32 i = 0U; i < ticks; ++i) {
      const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
      for (UInt32 j = 0U; j < timersPerTick; ++j) {
        EXPECT_FALSE(timers[i][j].inTimingWheel());
        timers[i][j].setCleanupHandler(&cleanupHandler);
        Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds, now);
        EXPECT_EQ(ESB_SUCCESS, error);
        EXPECT_TRUE(timers[i][j].inTimingWheel());
      }
    }
  }

  // cleanup handler should have been called 15 times
  EXPECT_EQ(ticks * timersPerTick, cleanupHandler.calls());
}

TEST(TimingWheelTest, AddTimeToTimer) {
  const UInt32 ticks = 5;
  const UInt32 timersPerTick = 3;
  const UInt32 tickMilliSeconds = 1;
  Date now(Time::Instance().now().seconds(), 0);
  const Date timeIncrement(0, tickMilliSeconds * 1000);
  Timer timers[ticks][timersPerTick];
  FlatTimingWheel timingWheel(ticks * 2 + 1, tickMilliSeconds, now);

  // Schedule

  for (UInt32 i = 0U; i < ticks; ++i) {
    const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      EXPECT_FALSE(timers[i][j].inTimingWheel());
      Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds, now);
      EXPECT_EQ(ESB_SUCCESS, error);
      EXPECT_TRUE(timers[i][j].inTimingWheel());
    }
  }

  // Initially nothing expires
  Timer *timer = timingWheel.nextExpired(now);
  EXPECT_EQ(NULL, timer);

  // Delay 1/3rd of the timers

  for (UInt32 i = 0U; i < ticks; ++i) {
    const UInt32 newExpirationMilliSeconds = (i + 1) * tickMilliSeconds + (ticks * tickMilliSeconds);
    Error error = timingWheel.update(&timers[i][1], newExpirationMilliSeconds, now);
    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_TRUE(timers[i][0].inTimingWheel());
    EXPECT_TRUE(timers[i][1].inTimingWheel());
    EXPECT_TRUE(timers[i][2].inTimingWheel());
  }

  // Then exactly 2 timers should expire every tick/millisecond

  for (UInt32 i = 0U; i < ticks; ++i) {
    now += timeIncrement;
    for (UInt32 j = 0U; j < timersPerTick - 1; ++j) {
      timer = timingWheel.nextExpired(now);
      EXPECT_TRUE(timer);
      EXPECT_FALSE(timer->inTimingWheel());
    }
    timer = timingWheel.nextExpired(now);
    EXPECT_EQ(NULL, timer);
  }

  // Then exactly 1 timer should expire every tick/millisecond, the ones we added delay to

  for (UInt32 i = 0U; i < ticks; ++i) {
    now += timeIncrement;
    timer = timingWheel.nextExpired(now);
    EXPECT_TRUE(timer);
    EXPECT_FALSE(timer->inTimingWheel());
    timer = timingWheel.nextExpired(now);
    EXPECT_EQ(NULL, timer);
  }

  // Then nothing

  now += timeIncrement;
  timer = timingWheel.nextExpired(now);
  EXPECT_EQ(NULL, timer);
}

TEST(TimingWheelTest, RemoveTimeFromTimer) {
  const UInt32 ticks = 5;
  const UInt32 timersPerTick = 3;
  const UInt32 tickMilliSeconds = 1;
  Date now(Time::Instance().now().seconds(), 0);
  const Date timeIncrement(0, tickMilliSeconds * 1000);
  Timer timers[ticks][timersPerTick];
  FlatTimingWheel timingWheel(ticks * 2 + 1, tickMilliSeconds, now);

  // Schedule

  for (UInt32 i = 0U; i < ticks; ++i) {
    const UInt32 expirationMilliSeconds = (i + 1) * tickMilliSeconds;
    for (UInt32 j = 0U; j < timersPerTick; ++j) {
      EXPECT_FALSE(timers[i][j].inTimingWheel());
      Error error = timingWheel.insert(&timers[i][j], expirationMilliSeconds, now);
      EXPECT_EQ(ESB_SUCCESS, error);
      EXPECT_TRUE(timers[i][j].inTimingWheel());
    }
  }

  // Initially nothing expires
  Timer *timer = timingWheel.nextExpired(now);
  EXPECT_EQ(NULL, timer);

  // Promote 1/3rd of the timers... this effectively removes them from the timing wheel

  for (UInt32 i = 0U; i < ticks; ++i) {
    EXPECT_TRUE(timers[i][1].inTimingWheel());
    const UInt32 newExpirationMilliSeconds = 0U;
    Error error = timingWheel.update(&timers[i][1], newExpirationMilliSeconds, now);
    EXPECT_EQ(ESB_UNDERFLOW, error);
    EXPECT_TRUE(timers[i][0].inTimingWheel());
    EXPECT_FALSE(timers[i][1].inTimingWheel());
    EXPECT_TRUE(timers[i][2].inTimingWheel());
  }

  // Then exactly 2 timers should expire every tick/millisecond

  for (UInt32 i = 0U; i < ticks; ++i) {
    now += timeIncrement;
    for (UInt32 j = 0U; j < timersPerTick - 1; ++j) {
      timer = timingWheel.nextExpired(now);
      EXPECT_TRUE(timer);
    }
    timer = timingWheel.nextExpired(now);
    EXPECT_EQ(NULL, timer);
  }

  // Then nothing

  now += timeIncrement;
  timer = timingWheel.nextExpired(now);
  EXPECT_EQ(NULL, timer);
}