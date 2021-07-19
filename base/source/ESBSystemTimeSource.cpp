#ifndef ESB_SYSTEM_TIME_SOURCE_H
#include <ESBSystemTimeSource.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

namespace ESB {

SystemTimeSource SystemTimeSource::_Instance;

SystemTimeSource::SystemTimeSource() {}
SystemTimeSource::~SystemTimeSource() {}

Date SystemTimeSource::now() { return SystemTimeSource::Now(); }

Date SystemTimeSource::Now() {
#if defined HAVE_GETTIMEOFDAY && defined HAVE_STRUCT_TIMEVAL
  struct timeval tv;
  gettimeofday(&tv, 0);
  return Date(tv.tv_sec, tv.tv_usec);

#elif defined HAVE_GET_SYSTEM_TIME_AS_FILE_TIME && defined HAVE_FILETIME && defined HAVE_32_X_32_TO_64
  FILETIME fileTime;
  GetSystemTimeAsFileTime(&fileTime);
  Int64 now = Int32x32To64(fileTime.dwLowDateTime, fileTime.dwHighDateTime) - ESB_INT64_C(116444736000000000);

    return Date((UInt32)(now / ESB_INT64_C(10000000)), (UInt32)((now % ESB_INT64_C(10000000)) / ESB_INT64_C(10));
#else
#error "gettimeofday or equivalent is required"
#endif
}

}  // namespace ESB
