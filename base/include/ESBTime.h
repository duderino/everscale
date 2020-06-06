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
    Date now(_Basis + _time.get(), 0);
    return now;
  }

  /** Get the current system time in seconds.
   *
   *  @return system time in seconds.
   */
  inline UInt64 nowSec() { return ((UInt64)_Basis) + _time.get(); }

  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 protected:
  virtual void run();

 private:
  //  Disabled
  Time();
  Time(const Time &);
  Time &operator=(const Time &);

  static Time _Instance;
  static UInt32 _Basis;
  SharedInt _time;
};

}  // namespace ESB

#endif
