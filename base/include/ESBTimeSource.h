#ifndef ESB_TIME_SOURCE_H
#define ESB_TIME_SOURCE_H

#ifndef ESB_DATE_H
#include <ESBDate.h>
#endif

namespace ESB {

/** TimeSource is an interface for determining the current time.
 *
 *  @ingroup util
 */
class TimeSource {
 public:
  /** Constructor */
  TimeSource();

  /** Destructor. */
  virtual ~TimeSource();

  /** Get a new date object initialized to the current time
   *
   *  @return date object set to the current time.
   */
  virtual Date now() = 0;

  ESB_DISABLE_AUTO_COPY(TimeSource);
};

class FakeTimeSource : public TimeSource {
 public:
  FakeTimeSource(const Date &date = Date(0, 0));

  virtual ~FakeTimeSource();

  virtual Date now();

  inline void setNow(const Date &now) { _now = now; }

  inline void addSeconds(UInt32 seconds) { _now += seconds; }

  inline void addMilliSeconds(UInt32 milliSeconds) { add(Date(milliSeconds / 1000, milliSeconds % 1000 * 1000)); }

  inline void add(const Date date) { _now += date; }

 private:
  Date _now;

  ESB_DISABLE_AUTO_COPY(FakeTimeSource);
};

}  // namespace ESB

#endif
