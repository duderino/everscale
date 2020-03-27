#ifndef ESB_TIME_SERIES_H
#define ESB_TIME_SERIES_H

#ifndef ESB_PERFORMANCE_COUNTER_H
#include <ESBPerformanceCounter.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ESB {

/**
 * A sequence of performance counters equally spaced in time.
 */
class TimeSeries : public PerformanceCounter {
 public:
  /**
   * Constructor.
   *
   * @param name The name of the time series counter
   * @param maxWindows The max windows this counter will store.  Once exceeded,
   *   it will free the oldest window in order to make room for the new one.
   * @param windowSizeSec The time between measurements in the sequence.
   * @param allocator The counter will grab memory from this allocator
   *   for each new element in the sequence.
   *
   */
  TimeSeries(const char *name, UInt16 maxWindows, UInt16 windowSizeSec,
             Allocator &allocator = SystemAllocator::Instance());

  virtual ~TimeSeries();

  virtual void record(const Date &start, const Date &stop);

  inline const char *name() const { return _name; }

  /**
   * Get a list of SimplePerformanceCounters.  There will be one counter in
   * the list per window that actually had observations.
   *
   * @return The list of SimplePerformanceCounters.
   */
  inline const EmbeddedList *counters() const { return &_list; }

  virtual void log(Logger &logger, Logger::Severity severity) const;

  inline void *operator new(size_t size, Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 private:
  // Disabled
  TimeSeries(const TimeSeries &counter);
  TimeSeries *operator=(const TimeSeries &counter);

  UInt16 _currentWindows;
  const UInt16 _maxWindows;
  const UInt16 _windowSizeSec;
  const char *_name;
  mutable Mutex _lock;
  EmbeddedList _list;
  Allocator &_allocator;
};

}  // namespace ESB

#endif
