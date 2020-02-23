#ifndef ESB_SIMPLE_PERFORMANCE_COUNTER_H
#define ESB_SIMPLE_PERFORMANCE_COUNTER_H

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_PERFORMANCE_COUNTER_H
#include <ESBPerformanceCounter.h>
#endif

namespace ESB {

class SimplePerformanceCounter : public PerformanceCounter {
 public:
  /**
   * Create a new counter with no stop or start time.
   *
   * @param name The counter's name
   */
  SimplePerformanceCounter(const char *name);

  /**
   * Create a new counter with a start and a stop time.
   *
   * @param name The counter's name
   * @param startTime the start time of the counter's window (number of seconds
   * since the epoch)
   * @param stopTime The stop time of the counter's window (number of seconds
   * since the epoch).
   */
  SimplePerformanceCounter(const char *name, time_t startTime, time_t stopTime);

  virtual ~SimplePerformanceCounter();

  virtual void addObservation(const struct timeval *start);

  virtual void addObservation(const struct timeval *start,
                              const struct timeval *stop);

  virtual void printSummary(FILE *file) const;

  inline const char *getName() const { return _name; }

  inline time_t getStartTime() const { return _startTime; }

  inline time_t getStopTime() const { return _stopTime; }

  inline unsigned long getThroughput() const { return _throughput; }

  inline double getAverageLatencyMsec() const { return _avgLatencyMsec; }

  inline double getMinLatencyMsec() const {
    return 0 > _minLatencyMsec ? 0 : _minLatencyMsec;
  }

  inline double getMaxLatencyMsec() const { return _maxLatencyMsec; }

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
  SimplePerformanceCounter(const SimplePerformanceCounter &counter);
  SimplePerformanceCounter *operator=(const SimplePerformanceCounter &counter);

  const time_t _startTime;
  const time_t _stopTime;
  const char *_name;
  double _avgLatencyMsec;
  double _minLatencyMsec;
  double _maxLatencyMsec;
  unsigned long _throughput;
  mutable Mutex _lock;
};

}  // namespace ESB

#endif
