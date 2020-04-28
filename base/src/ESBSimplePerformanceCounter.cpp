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
      _latencyMsec(),
      _lock() {}

SimplePerformanceCounter::SimplePerformanceCounter(const char *name,
                                                   const Date &windowStart,
                                                   const Date &windowStop)
    : PerformanceCounter(),
      _name(name),
      _windowStart(windowStart),
      _windowStop(windowStop),
      _latencyMsec(),
      _lock() {}

SimplePerformanceCounter::~SimplePerformanceCounter() {}

double SimplePerformanceCounter::queriesPerSec() const {
  ReadScopeLock lock(_lock);
  return queriesPerSecNoLock();
}

double SimplePerformanceCounter::queriesPerSecNoLock() const {
  if (0 == _latencyMsec.n()) {
    return 0;
  }

  Date window;

  if (0 == _windowStop.seconds() && 0 == _windowStop.microSeconds()) {
    window = Date::Now() - _windowStart;
  } else {
    window = _windowStop - _windowStart;
  }

  double diffMsec = window.seconds() * 1000.0 + window.microSeconds() / 1000.0;

  return _latencyMsec.n() / diffMsec * 1000.0;
}

void SimplePerformanceCounter::record(const Date &start, const Date &stop) {
  Date diff(stop - start);
  double diffMSec = diff.seconds() * 1000.0 + diff.microSeconds() / 1000.0;

  {
    WriteScopeLock lock(_lock);

    if (0 == _windowStart.seconds() && 0 == _windowStart.microSeconds()) {
      _windowStart = Date::Now();
    }

    _latencyMsec.add(diffMSec);
  }
}

double SimplePerformanceCounter::varianceMSec() const {
  ReadScopeLock lock(_lock);
  return _latencyMsec.variance();
}

void SimplePerformanceCounter::log(Logger &logger,
                                   Logger::Severity severity) const {
  double meanMSec, varianceMSec, minMSec, maxMSec, qps = 0.0;
  UInt32 queries = 0U;

  {
    ReadScopeLock lock(_lock);

    meanMSec = _latencyMsec.mean();
    varianceMSec = _latencyMsec.variance();
    minMSec = _latencyMsec.min();
    maxMSec = _latencyMsec.max();
    qps = queriesPerSecNoLock();
    queries = _latencyMsec.n();
  }

  ESB_LOG(logger, severity,
          "%s: %.2lf, %u, %.4lf, %.4f, %.4lf, %.4f",
          _name, qps, queries, meanMSec, varianceMSec, minMSec, maxMSec);
}

UInt32 SimplePerformanceCounter::queries() const {
  ReadScopeLock lock(_lock);
  return _latencyMsec.n();
}

double SimplePerformanceCounter::meanMSec() const {
  ReadScopeLock lock(_lock);
  return _latencyMsec.mean();
}

double SimplePerformanceCounter::minMSec() const {
  ReadScopeLock lock(_lock);
  return _latencyMsec.min();
}

double SimplePerformanceCounter::maxMSec() const {
  ReadScopeLock lock(_lock);
  return _latencyMsec.max();
}

}  // namespace ESB
