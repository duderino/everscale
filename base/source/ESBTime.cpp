#ifndef ESB_TIME_H
#include <ESBTime.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

namespace ESB {

Time Time::_Instance;

Time::Time() : Thread(), _basis(Date::Now()), _time(0), _resolutionMilliSeconds(100) {}

Time::~Time() {}

void Time::run() {
  while (_isRunning.get()) {
    Date now = Date::Now() - _basis;
    ESB::Int32 milliSeconds = now.seconds() * 1000 + now.microSeconds() / 1000;

    if (0 > milliSeconds) {
      ESB_LOG_CRITICAL("Process exceeded max millisecond lifetime");
      return;
    }

    _time.set(milliSeconds);

#ifdef HAVE_USLEEP
    usleep(_resolutionMilliSeconds * 1000);
#endif
  }
}

}  // namespace ESB
