#ifndef ESB_TIME_H
#define ESB_TIME_H

#ifndef ESB_DATE_H
#include <ESBDate.h>
#endif

#ifndef ESB_THREAD_H
#include <ESBThread.h>
#endif

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

namespace ESB {

/** @defgroup util Utilities */

/** Time is a second resolution clock that caches time-related system calls.
 *
 *  @ingroup util
 */
class Time : public Thread {
 public:
  static inline Time &Instance() { return _Instance; }

  /** Default destructor. */
  virtual ~Time();

  /** Get a new date object initialized to the current system time, truncated
   *  to the second.
   *
   *  @return date object set to the current time, truncated to the second.
   */
  inline Date now() {
    Int32 milliSeconds = _time.get();
    Date now;
    now.setMicroSeconds(_basis.microSeconds() + milliSeconds * 1000);
    now.setSeconds(_basis.seconds() + milliSeconds / 1000);
    return now;
  }

  inline UInt32 resolutionMilliSeconds() const { return _resolutionMilliSeconds; }

  inline void setResolutionMilliSeconds(UInt32 resolutionMilliSeconds) {
    _resolutionMilliSeconds = resolutionMilliSeconds;
  }

  /** Get the current system time in seconds.
   *
   *  @return system time in seconds.
   */
  inline UInt64 nowSec() { return now().seconds(); }

  /**
   * Used by time-based test cases
   *
   * @param seconds Set the global system time.
   */
  inline void setNow(UInt32 milliSeconds) { _time.set(milliSeconds); }

  /**
   * Used by time-based test cases
   *
   * @param seconds Set the global system time.
   */
  inline void addNow(UInt32 milliSeconds) { _time.add(milliSeconds); }

  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 protected:
  virtual void run();

 private:
  //  Disabled
  Time();
  Time(const Time &);
  Time &operator=(const Time &);

  Date _basis;
  SharedInt _time;
  ESB::UInt32 _resolutionMilliSeconds;
  static Time _Instance;
};

}  // namespace ESB

#endif
