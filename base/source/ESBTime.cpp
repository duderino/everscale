#ifndef ESB_TIME_H
#include <ESBTime.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

namespace ESB {

Time Time::_Instance;
UInt32 Time::_Basis = 946641600;  // second since epoch at turn of millennium

Time::Time() : Thread() {
#ifdef HAVE_TIME
  _time.set((int)(time(0) - _Basis));
#else
#error "time() or equivalent is required"
#endif
}

Time::~Time() {}

void Time::run() {
  while (_isRunning.get()) {
#ifdef HAVE_TIME
    _time.set((int)(time(0) - _Basis));
#else
#error "time() or equivalent is required"
#endif

#ifdef HAVE_USLEEP
    usleep(333 * 1000);
#endif
  }
}

}  // namespace ESB
