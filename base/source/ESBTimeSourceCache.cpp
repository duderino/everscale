#ifndef ESB_TIME_SOURCE_CACHE_H
#include <ESBTimeSourceCache.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

namespace ESB {

TimeSourceCache::TimeSourceCache(TimeSource &source, UInt32 updateMilliSeconds)
    : Thread(), _basis(source.now()), _time(0), _updateMilliSeconds(updateMilliSeconds), _source(source) {}

TimeSourceCache::~TimeSourceCache() {}

void TimeSourceCache::run() {
  while (_isRunning.get()) {
    Date now = _source.now() - _basis;
    _time.set(now.seconds());

#ifdef HAVE_USLEEP
    usleep(_updateMilliSeconds * 1000);
#endif
  }
}

}  // namespace ESB
