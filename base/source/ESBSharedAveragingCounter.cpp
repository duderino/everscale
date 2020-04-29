#ifndef ESB_SHARED_AVERAGING_COUNTER_H
#include <ESBSharedAveragingCounter.h>
#endif

#ifndef ESB_READ_SCOPE_LOCK_H
#include <ESBReadScopeLock.h>
#endif

#ifndef EBS_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

namespace ESB {

SharedAveragingCounter::SharedAveragingCounter() : _counter(), _lock() {}

SharedAveragingCounter::~SharedAveragingCounter() {}

void SharedAveragingCounter::log(Logger &logger, Logger::Severity severity,
                                 const char *description) const {
  UInt32 n = 0;
  double mean, variance, min, max = 0.0;

  {
    ReadScopeLock lock(_lock);
    n = _counter.n();
    mean = _counter.mean();
    variance = _counter.variance();
    min = _counter.min();
    max = _counter.max();
  }

  ESB_LOG(logger, severity,
          "%s: N=%u, MEAN=%.2lf, VAR=%.2f, MIN=%.2lf, MAX=%.2lf", description,
          n, mean, variance, min, max);
}

}  // namespace ESB
