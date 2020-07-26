#ifndef ESB_DATE_H
#include "ESBDate.h"
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

namespace ESB {

/** Default destructor. */
Date::~Date() {}

/** Plus equals operator.
 *
 *  @param date The date to add to this date.
 *  @return this object.
 */
Date &Date::operator+=(const Date &date) {
  _seconds += date._seconds;
  _seconds += (_microSeconds + date._microSeconds) / ESB_UINT32_C(1000000);
  _microSeconds = (_microSeconds + date._microSeconds) % ESB_UINT32_C(1000000);
  return *this;
}

/** Minus equals operator.
 *
 *    @param date The date to subtract from this date.
 *    @return this object.
 */
Date &Date::operator-=(const Date &date) {
  if (date._microSeconds > _microSeconds) {
    --_seconds;
    _microSeconds += 1000 * 1000;
  }
  _seconds -= date._seconds;
  _microSeconds -= date._microSeconds;
  return *this;
}

}  // namespace ESB
