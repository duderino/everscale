#ifndef ESB_TIME_H
#define ESB_TIME_H

#ifndef ESB_TIME_SOURCE_H
#include <ESBTimeSource.h>
#endif

namespace ESB {

/** Time is a second resolution clock that caches time-related system calls.
 *
 *  @ingroup util
 */
class Time {
 public:
  /** Constructor */
  static inline Time &Instance() { return _Instance; }

  /** Destructor. */
  virtual ~Time();

  /** Get a new date object initialized to the current time
   *
   *  @return date object set to the current time.
   */
  inline Date now() { return _source->now(); }

  inline void setTimeSource(TimeSource &source) { _source = &source; }

 private:
  //  Disabled
  Time();
  Time(const Time &);
  Time &operator=(const Time &);

  TimeSource *_source;
  static Time _Instance;
};

}  // namespace ESB

#endif
