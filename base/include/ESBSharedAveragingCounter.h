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

  inline double getMean() const {
    ReadScopeLock lock(_lock);
    return _counter.getMean();
  }

  inline double getVariance() const {
    ReadScopeLock lock(_lock);
    return _counter.getVariance();
  }

  inline double getMin() const {
    ReadScopeLock lock(_lock);
    return _counter.getMin();
  }

  inline double getMax() const {
    ReadScopeLock lock(_lock);
    return _counter.getMax();
  }

  inline UInt32 getObservations() const {
    ReadScopeLock lock(_lock);
    return _counter.getObservations();
  }

  void log(Logger &logger, Logger::Severity severity,
           const char *description) const;

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  // Disabled
  SharedAveragingCounter(const SharedAveragingCounter &counter);
  SharedAveragingCounter &operator=(const SharedAveragingCounter &counter);

  AveragingCounter _counter;
  mutable Mutex _lock;
};

}  // namespace ESB

#endif
