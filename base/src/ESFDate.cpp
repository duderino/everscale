/** @file ESFDate.cpp
 *  @brief A microsecond-resolution clock
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_DATE_H
#include "ESFDate.h"
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

/** Default destructor. */
ESFDate::~ESFDate() {}

/** Plus equals operator.
 *
 *  @param date The date to add to this date.
 *  @return this object.
 */
ESFDate &ESFDate::operator+=(const ESFDate &date) {
  _seconds += date._seconds;

  _seconds += (_microSeconds + date._microSeconds) / ESF_UINT32_C(1000000);
  _microSeconds = (_microSeconds + date._microSeconds) % ESF_UINT32_C(1000000);

  return *this;
}

/** Minus equals operator.
 *
 *    @param date The date to subtract from this date.
 *    @return this object.
 */
ESFDate &ESFDate::operator-=(const ESFDate &date) {
  _seconds -= date._seconds;
  _microSeconds -= date._microSeconds;

  if (0 > _microSeconds) {
    --_seconds;
    _microSeconds += ESF_UINT32_C(1000000);
  }

  return *this;
}

/** Get a new date object initialized to the current system time.
 *
 *    @return date object set to the current time.
 */
ESFDate ESFDate::GetSystemTime() {
  ESFDate currentTime;

#if defined HAVE_GETTIMEOFDAY && defined HAVE_STRUCT_TIMEVAL

  struct timeval tv;

  gettimeofday(&tv, 0);

  currentTime._seconds = tv.tv_sec;
  currentTime._microSeconds = tv.tv_usec;

#elif defined HAVE_GET_SYSTEM_TIME_AS_FILE_TIME && defined HAVE_FILETIME && \
    defined HAVE_32_X_32_TO_64

  FILETIME fileTime;

  GetSystemTimeAsFileTime(&fileTime);

  ESFInt64 now = Int32x32To64(fileTime.dwLowDateTime, fileTime.dwHighDateTime) -
                 ESF_INT64_C(116444736000000000);

  currentTime._seconds = (ESFUInt32)(now / ESF_INT64_C(10000000));
  currentTime._microSeconds =
      (ESFUInt32)((now % ESF_INT64_C(10000000)) / ESF_INT64_C(10));

#else
#error "gettimeofday or equivalent is required"
#endif

  return currentTime;
}
