#ifndef ESB_SIMPLE_PERFORMANCE_COUNTER_H
#define ESB_SIMPLE_PERFORMANCE_COUNTER_H

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_PERFORMANCE_COUNTER_H
#include <ESBPerformanceCounter.h>
#endif

#ifndef ESB_AVERAGING_COUNTER_H
#include <ESBAveragingCounter.h>
#endif

namespace ESB {

class SimplePerformanceCounter : public PerformanceCounter {
 public:
  /**
   * Create a new counter with a dynamically determined window.
   *
   * @param name The counter's name
   */
  explicit SimplePerformanceCounter(const char *name);

  /**
   * Create a new counter with a pre-defined window.
   *
   * @param name The counter's name
   * @param windowStart the start time of the counter's window
   * @param windowStop The stop time of the counter's window
   */
  SimplePerformanceCounter(const char *name, const Date &windowStart,
                           const Date &windowStop);

  virtual ~SimplePerformanceCounter();

  inline const char *name() const { return _name; }

  inline const Date &windowStart() const { return _windowStart; }

  inline const Date &windowStop() const { return _windowStop; }

  double queriesPerSec() const;

  UInt32 queries() const;

  double meanMSec() const;

  double varianceMSec() const;

  double minMSec() const;

  double maxMSec() const;

  virtual void record(const Date &start, const Date &stop);

  virtual void log(Logger &logger, Logger::Severity severity) const;

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

  inline void *operator new(size_t size,
                            SimplePerformanceCounter *counter) noexcept {
    return counter;
  }

 private:
  // Disabled
  SimplePerformanceCounter(const SimplePerformanceCounter &counter);
  SimplePerformanceCounter *operator=(const SimplePerformanceCounter &counter);

  double queriesPerSecNoLock() const;

  const char *_name;
  Date _windowStart;
  Date _windowStop;
  AveragingCounter _latencyMsec;
  mutable Mutex _lock;
};

}  // namespace ESB

#endif
