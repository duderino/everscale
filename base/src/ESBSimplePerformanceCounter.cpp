#ifndef ESB_SIMPLE_PERFORMANCE_COUNTER_H
#include <ESBSimplePerformanceCounter.h>
#endif

#ifndef ESB_READ_SCOPE_LOCK_H
#include <ESBReadScopeLock.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

namespace ESB {

SimplePerformanceCounter::SimplePerformanceCounter(const char *name)
    : PerformanceCounter(),
      _name(name),
      _windowStart(),
      _windowStop(),
      _avgMSec(0.0),
      _minMSec(-1.0),
      _maxMSec(0.0),
      _queries(0UL),
      _lock() {}

SimplePerformanceCounter::SimplePerformanceCounter(const char *name,
                                                   const Date &windowStart,
                                                   const Date &windowStop)
    : PerformanceCounter(),
      _name(name),
      _windowStart(windowStart),
      _windowStop(windowStop),
      _avgMSec(0.0),
      _minMSec(-1.0),
      _maxMSec(0.0),
      _queries(0UL),
      _lock() {}

SimplePerformanceCounter::~SimplePerformanceCounter() {}

UInt32 SimplePerformanceCounter::getQueriesPerSec() const {
  ReadScopeLock lock(_lock);

  if (0 == _queries) {
    return 0;
  }

  Date windowStop(0 == _windowStop.getSeconds() &&
                          0 == _windowStop.getMicroSeconds()
                      ? Date::GetSystemTime()
                      : _windowStop);

  UInt32 windowSec = (windowStop - _windowStart).getSeconds();

  if (0 == windowSec) {
    return 0;
  }

  return _queries / windowSec;
}

void SimplePerformanceCounter::addObservation(const Date &start,
                                              const Date &stop) {
  Date diff(stop - start);
  UInt32 diffMSec = diff.getSeconds() * 1000 + diff.getMicroSeconds() / 1000;

  {
    WriteScopeLock lock(_lock);

    double n = ++_queries;

    _avgMSec = (diffMSec * (1.0 / n)) + (_avgMSec * ((n - 1.0) / n));

    if (0 == _windowStart.getSeconds() && 0 == _windowStart.getMicroSeconds()) {
      _windowStart = Date::GetSystemTime();
    }

    if (0 > _minMSec) {
      _minMSec = diffMSec;
    } else if (_minMSec > diffMSec) {
      _minMSec = diffMSec;
    }

    if (diffMSec > _maxMSec) {
      _maxMSec = diffMSec;
    }
  }
}

void SimplePerformanceCounter::printSummary(FILE *file) const {
  _lock.readAcquire();

  double avgMSec = _avgMSec;
  double minMSec = _minMSec;
  double maxMSec = _maxMSec;
  UInt32 qps = getQueriesPerSec();

  _lock.readRelease();

  fprintf(file,
          "%s: RPS=%" PRIu32
          ", AVG LAT MSEC=%lf, MIN LAT MSEC=%lf, MAX LAT MSEC=%lf\n",
          _name, qps, avgMSec, minMSec, maxMSec);
}

}  // namespace ESB
