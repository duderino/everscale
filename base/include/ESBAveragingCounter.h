#ifndef ESB_AVERAGING_COUNTER_H
#define ESB_AVERAGING_COUNTER_H

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ESB {

/**
 * An unsynchronized counter that computes the mean and variance of a series of
 * values.
 */
class AveragingCounter {
 public:
  AveragingCounter();

  virtual ~AveragingCounter();

  void add(double value);

  inline double getMean() const { return _mean; }

  inline double getVariance() const {
    return 1 >= _observations ? 0.0 : _avgDistToMeanSq / (_observations - 1);
  }

  inline double getMin() const { return _min; }

  inline double getMax() const { return _max; }

  inline UInt32 getObservations() const { return _observations; }

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
  AveragingCounter(const AveragingCounter &counter);
  void operator=(const AveragingCounter &counter);

  double _mean;
  double _avgDistToMeanSq;
  double _min;
  double _max;
  UInt32 _observations;
};

}  // namespace ESB

#endif
