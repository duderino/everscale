#ifndef ESB_FLAT_TIMING_WHEEL_H
#define ESB_FLAT_TIMING_WHEEL_H

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_TIMER_H
#include <ESBTimer.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_DATE_H
#include <ESBDate.h>
#endif

namespace ESB {

/** A flat timing wheel for O(1) insertion, bookkeeping and removal at the expense of more memory usage which in turn
 * favors narrower windows / max delays.  See "Hashed and Hierarchical Timing Wheels: Data Structures
for the Efficient Implementation of a Timer Facility" by Varghese et al.
 *
 *  @ingroup util
 */
class FlatTimingWheel {
 public:
  /** Constructor.
   */
  FlatTimingWheel(UInt32 ticks, UInt32 tickMilliSeconds, const Date &now,
                  Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~FlatTimingWheel();

  /** Insert/schedule a timer.  O(1).
   *
   *  NB: both underflows and overflows can unexpectedly occur if remove() is not called frequently enough.  Only
   * remove() advances the timing wheel's view of the current time.
   *
   *  @param timer The timer to activate after a delay.
   *  @param delayMilliSeconds the milliseconds to wait before executing the command
   *  @param now The current time
   *  @return ESB_SUCCESS if successful, ESB_OVERFLOW if delay too far into the future, ESB_UNDERFLOW if delay has
   * already expired - caller should act like the timer just expired.
   */
  Error insert(Timer *timer, UInt32 delayMilliSeconds, const Date &now);

  /** Update a scheduled timer.  O(1) but corruption will occur if the timer is not in the timing wheel.
   *
   *  @param timer The timer to activate after a delay.
   *  @param delayMilliSeconds the milliseconds to wait before executing the command
   *  @param now The current time
   *  @return ESB_SUCCESS if successful, ESB_OVERFLOW if delay too far into the future, ESB_UNDERFLOW if delay has
   * already past - caller should act like the timer just expired and the timer will be removed as a side effect.
   */
  Error update(Timer *timer, UInt32 delayMilliSeconds, const Date &now);

  /**
   * Remove/cancel a timer.  O(1) but corruption will occur if the timer is not in the timing wheel.
   *
   * @param timer The timer to remove/cancel.
   * @return ESB_SUCCESS if successful, another error code otherwise.  Always succeeds unless timer is NULL.
   */
  Error remove(Timer *timer);

  /**
   * Remove an expired timer from the timing wheel.  Caller should keep calling this until it returns null.
   *
   * @param now The current time
   * @return A timer which has expired, or NULL if there are no more expired timers.
   */
  Timer *nextExpired(const Date &now);

  /**
   * Remove all timers from the timing wheel, calling their cleanup handlers in the process.
   */
  void clear();

  inline UInt32 maxDelayMilliSeconds() { return _tickMilliSeconds * _maxTicks; }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  // Disabled
  FlatTimingWheel(const FlatTimingWheel &);
  FlatTimingWheel &operator=(const FlatTimingWheel &);

  inline UInt32 idx(Int32 value) { return (value % _maxTicks + _maxTicks) % _maxTicks; }

  inline UInt32 ticks(Date date) const {
    UInt32 ticks = date.seconds() * 1000 / _tickMilliSeconds;
    ticks += date.microSeconds() / 1000 / _tickMilliSeconds;
    return ticks;
  }

  const Date _start;
  const UInt32 _tickMilliSeconds;
  const UInt32 _maxTicks;  // 1 bucket per tick
  UInt32 _currentTick;     // relative to _start, and in ticks
  EmbeddedList *_timers;
  Allocator &_allocator;
};

}  // namespace ESB

#endif
