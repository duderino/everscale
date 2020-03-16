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

double SimplePerformanceCounter::getQueriesPerSec() const {
  ReadScopeLock lock(_lock);
  return getQueriesPerSecNoLock();
}

double SimplePerformanceCounter::getQueriesPerSecNoLock() const {
  if (0 == _latencyMsec.getObservations()) {
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

  return _latencyMsec.getObservations() / diffMsec * 1000.0;
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

    _latencyMsec.add(diffMSec);
  }
}

double SimplePerformanceCounter::getVarianceMsec() const {
  ReadScopeLock lock(_lock);
  return _latencyMsec.getVariance();
}

void SimplePerformanceCounter::log(Logger &logger,
                                   Logger::Severity severity) const {
  double meanMSec, varianceMSec, minMSec, maxMSec, qps = 0.0;
  UInt32 queries = 0U;

  {
    ReadScopeLock lock(_lock);

    meanMSec = _latencyMsec.getMean();
    varianceMSec = _latencyMsec.getVariance();
    minMSec = _latencyMsec.getMin();
    maxMSec = _latencyMsec.getMax();
    qps = getQueriesPerSecNoLock();
    queries = _latencyMsec.getObservations();
  }

  ESB_LOG(logger, severity,
          "%s: QPS=%.2lf, N=%u, LATENCY MSEC MEAN=%.2lf, VAR=%.2f, MIN=%.2lf, "
          "MAX=%.2lf",
          _name, qps, queries, meanMSec, varianceMSec, minMSec, maxMSec);
}

UInt32 SimplePerformanceCounter::getQueries() const {
  ReadScopeLock lock(_lock);
  return _latencyMsec.getObservations();
}

double SimplePerformanceCounter::getMeanMsec() const {
  ReadScopeLock lock(_lock);
  return _latencyMsec.getMean();
}

double SimplePerformanceCounter::getMinMsec() const {
  ReadScopeLock lock(_lock);
  return _latencyMsec.getMin();
}

double SimplePerformanceCounter::getMaxMsec() const {
  ReadScopeLock lock(_lock);
  return _latencyMsec.getMax();
}

}  // namespace ESB
