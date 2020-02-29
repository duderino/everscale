#ifndef ESB_HISTORICAL_PERFORMANCE_COUNTER_H
#define ESB_HISTORICAL_PERFORMANCE_COUNTER_H

#ifndef ESB_PERFORMANCE_COUNTER_H
#include <ESBPerformanceCounter.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ESB {

class HistoricalPerformanceCounter : public PerformanceCounter {
 public:
  /**
   * Constructor.
   *
   * @param name The name of the counter
   * @param windowSizeSec The counter's latency and throughput will be saved
   *   for each window of this many seconds.
   * @param allocator The counter will grab memory from this allocator
   *   for each new window it creates.
   */
  HistoricalPerformanceCounter(const char *name, UInt16 windowSizeSec,
                               Allocator *allocator, Logger *logger);

  virtual ~HistoricalPerformanceCounter();

  virtual void addObservation(const Date &start, const Date &stop);

  inline const char *getName() const { return _name; }

  /**
   * Get a list of SimplePerformanceCounters.  There will be one counter in
   * the list per window that actually had observations.
   *
   * @return The list of SimplePerformanceCounters.
   */
  inline const EmbeddedList *getCounters() const { return &_list; }

  virtual void printSummary(FILE *file) const;

 private:
  // Disabled
  HistoricalPerformanceCounter(const HistoricalPerformanceCounter &counter);
  HistoricalPerformanceCounter *operator=(
      const HistoricalPerformanceCounter &counter);

  UInt16 _windowSizeSec;
  const char *_name;
  Logger *_logger;
  EmbeddedList _list;
  mutable Mutex _lock;
  DiscardAllocator _allocator;
};

}  // namespace ESB

#endif
