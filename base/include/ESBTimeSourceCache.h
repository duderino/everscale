#ifndef ESB_TIME_SOURCE_CACHE_H
#define ESB_TIME_SOURCE_CACHE_H

#ifndef ESB_TIME_SOURCE_H
#include <ESBTimeSource.h>
#endif

#ifndef ESB_THREAD_H
#include <ESBThread.h>
#endif

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

namespace ESB {

/** @defgroup util Utilities */

/** TimeSourceCache is second-resolution cache for any TimeSource.
 *
 *  @ingroup util
 */
class TimeSourceCache : public Thread, public TimeSource {
 public:
  /**
   * Constructor
   *
   * @param source The time source to cached
   * @param updateMilliSeconds The background thread will update the time approximately every updateMilliSeconds
   */
  TimeSourceCache(TimeSource &source, UInt32 updateMilliSeconds = 333U);

  /** Default destructor. */
  virtual ~TimeSourceCache();

  virtual Date now() { return Date(_time.get() + _basis.seconds(), 0); }

  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 protected:
  virtual void run();

 private:
  //  Disabled
  TimeSourceCache(const TimeSourceCache &);
  TimeSourceCache &operator=(const TimeSourceCache &);

  Date _basis;
  SharedInt _time;
  ESB::UInt32 _updateMilliSeconds;
  TimeSource &_source;
};

}  // namespace ESB

#endif
