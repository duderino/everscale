#ifndef ESB_SHARED_AVERAGING_COUNTER_H
#define ESB_SHARED_AVERAGING_COUNTER_H

#ifndef ESB_AVERAGING_COUNTER_H
#include <ESBAveragingCounter.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

#ifndef ESB_READ_SCOPE_LOCK_H
#include <ESBReadScopeLock.h>
#endif

namespace ESB {

/**
 * An synchronized counter that computes the mean and variance of a series of
 * values.
 */
class SharedAveragingCounter {
 public:
  SharedAveragingCounter();

  virtual ~SharedAveragingCounter();

  inline void add(double value) {
    WriteScopeLock lock(_lock);
    return _counter.add(value);
  }

  inline double mean() const {
    ReadScopeLock lock(_lock);
    return _counter.mean();
  }

  inline double variance() const {
    ReadScopeLock lock(_lock);
    return _counter.variance();
  }

  inline double min() const {
    ReadScopeLock lock(_lock);
    return _counter.min();
  }

  inline double max() const {
    ReadScopeLock lock(_lock);
    return _counter.max();
  }

  inline UInt32 n() const {
    ReadScopeLock lock(_lock);
    return _counter.n();
  }

  void log(Logger &logger, Logger::Severity severity, const char *description) const;

 private:
  AveragingCounter _counter;
  mutable Mutex _lock;

  ESB_DEFAULT_FUNCS(SharedAveragingCounter);
};

}  // namespace ESB

#endif
