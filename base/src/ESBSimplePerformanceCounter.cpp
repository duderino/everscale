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

double SimplePerformanceCounter::getQueriesPerSec() const {
  ReadScopeLock lock(_lock);
  return getQueriesPerSecNoLock();
}

double SimplePerformanceCounter::getQueriesPerSecNoLock() const {
  if (0 == _queries) {
    return 0;
  }

  Date windowStop(0 == _windowStop.getSeconds() &&
                          0 == _windowStop.getMicroSeconds()
                      ? Date::Now()
                      : _windowStop);

  double windowSec = (windowStop - _windowStart).getSeconds();
  windowSec = ESB_MAX(windowSec, 1);

  return _queries / windowSec;
}

void SimplePerformanceCounter::addObservation(const Date &start,
                                              const Date &stop) {
  Date diff(stop - start);
  UInt32 diffMSec = diff.getSeconds() * 1000 + diff.getMicroSeconds();

  {
    WriteScopeLock lock(_lock);

    double n = ++_queries;

    _avgMSec = (diffMSec * (1.0 / n)) + (_avgMSec * ((n - 1.0) / n));

    if (0 == _windowStart.getSeconds() && 0 == _windowStart.getMicroSeconds()) {
      _windowStart = Date::Now();
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
  double avgMSec, minMSec, maxMSec, qps = 0.0;
  UInt32 queries = 0U;

  {
    ReadScopeLock lock(_lock);

    avgMSec = _avgMSec;
    minMSec = _minMSec;
    maxMSec = _maxMSec;
    qps = getQueriesPerSecNoLock();
    queries = _queries;
  }

  fprintf(file,
          "%s: QPS=%.2lf, N=%u, AVG LAT MSEC=%.2lf, MIN LAT MSEC=%.2lf, MAX "
          "LAT MSEC=%.2lf\n",
          _name, qps, queries, avgMSec, minMSec, maxMSec);
}

}  // namespace ESB
