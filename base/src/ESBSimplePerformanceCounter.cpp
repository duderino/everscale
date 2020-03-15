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
      _meanMSec(0.0),
      _avgDistToMeanSq(0.0),
      _minMSec(0.0),
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
      _meanMSec(0.0),
      _avgDistToMeanSq(0.0),
      _minMSec(0.0),
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

  Date window;

  if (0 == _windowStop.getSeconds() && 0 == _windowStop.getMicroSeconds()) {
    window = Date::Now() - _windowStart;
  } else {
    window = _windowStop - _windowStart;
  }

  double diffMsec =
      window.getSeconds() * 1000.0 + window.getMicroSeconds() / 1000.0;

  return _queries / diffMsec * 1000.0;
}

void SimplePerformanceCounter::addObservation(const Date &start,
                                              const Date &stop) {
  Date diff(stop - start);
  double diffMSec =
      diff.getSeconds() * 1000.0 + diff.getMicroSeconds() / 1000.0;

  {
    WriteScopeLock lock(_lock);

    if (0 == _windowStart.getSeconds() && 0 == _windowStart.getMicroSeconds()) {
      _windowStart = Date::Now();
    }

    // Welford's online algorithm
    double n = ++_queries;
    double delta = diffMSec - _meanMSec;
    _meanMSec += delta / n;
    double delta2 = diffMSec - _meanMSec;
    _avgDistToMeanSq += delta * delta2;

    if (0 >= _minMSec) {
      _minMSec = diffMSec;
    } else if (_minMSec > diffMSec) {
      _minMSec = diffMSec;
    }

    if (diffMSec > _maxMSec) {
      _maxMSec = diffMSec;
    }
  }
}

double SimplePerformanceCounter::getVarianceMsec() const {
  ReadScopeLock lock(_lock);
  return getVarianceMsecNoLock();
}

void SimplePerformanceCounter::printSummary(FILE *file) const {
  double meanMSec, varianceMSec, minMSec, maxMSec, qps = 0.0;
  UInt32 queries = 0U;

  {
    ReadScopeLock lock(_lock);

    meanMSec = _meanMSec;
    varianceMSec = getVarianceMsecNoLock();
    minMSec = _minMSec;
    maxMSec = _maxMSec;
    qps = getQueriesPerSecNoLock();
    queries = _queries;
  }

  fprintf(file,
          "%s: QPS=%.2lf, N=%u, LATENCY MSEC MEAN=%.2lf, VAR=%.2f, "
          "MIN=%.2lf, MAX =%.2lf\n",
          _name, qps, queries, meanMSec, varianceMSec, minMSec, maxMSec);
}
UInt32 SimplePerformanceCounter::getQueries() const {
  ReadScopeLock lock(_lock);
  return _queries;
}

double SimplePerformanceCounter::getMeanMsec() const {
  ReadScopeLock lock(_lock);
  return _meanMSec;
}

double SimplePerformanceCounter::getMinMsec() const {
  ReadScopeLock lock(_lock);
  return 0 > _minMSec ? 0 : _minMSec;
}

double SimplePerformanceCounter::getMaxMsec() const {
  ReadScopeLock lock(_lock);
  return _maxMSec;
}

double SimplePerformanceCounter::getVarianceMsecNoLock() const {
  if (1 >= _queries) {
    return 0.0;
  }

  return _avgDistToMeanSq / (_queries - 1);
}

}  // namespace ESB
