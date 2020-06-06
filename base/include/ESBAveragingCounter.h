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

  inline double mean() const { return _mean; }

  inline double variance() const { return 1 >= _n ? 0.0 : _avgDistToMeanSq / (_n - 1); }

  inline double min() const { return _min; }

  inline double max() const { return _max; }

  inline UInt32 n() const { return _n; }

  void log(Logger &logger, Logger::Severity severity, const char *description) const;

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  // Disabled
  AveragingCounter(const AveragingCounter &counter);
  void operator=(const AveragingCounter &counter);

  double _mean;
  double _avgDistToMeanSq;
  double _min;
  double _max;
  UInt32 _n;
};

}  // namespace ESB

#endif
