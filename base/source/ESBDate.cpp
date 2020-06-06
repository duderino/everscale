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

/** Get a new date object initialized to the current system time.
 *
 *    @return date object set to the current time.
 */
Date Date::Now() {
  Date currentTime;

#if defined HAVE_GETTIMEOFDAY && defined HAVE_STRUCT_TIMEVAL

  struct timeval tv;

  gettimeofday(&tv, 0);

  currentTime._seconds = tv.tv_sec;
  currentTime._microSeconds = tv.tv_usec;

#elif defined HAVE_GET_SYSTEM_TIME_AS_FILE_TIME && defined HAVE_FILETIME && defined HAVE_32_X_32_TO_64

  FILETIME fileTime;

  GetSystemTimeAsFileTime(&fileTime);

  Int64 now = Int32x32To64(fileTime.dwLowDateTime, fileTime.dwHighDateTime) - ESB_INT64_C(116444736000000000);

  currentTime._seconds = (UInt32)(now / ESB_INT64_C(10000000));
  currentTime._microSeconds = (UInt32)((now % ESB_INT64_C(10000000)) / ESB_INT64_C(10));

#else
#error "gettimeofday or equivalent is required"
#endif

  return currentTime;
}

}  // namespace ESB
