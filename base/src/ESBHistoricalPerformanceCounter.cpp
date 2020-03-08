#ifndef ESB_HISTORICAL_PERFORMANCE_COUNTER_H
#include <ESBHistoricalPerformanceCounter.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

#ifndef ESB_READ_SCOPE_LOCK_H
#include <ESBReadScopeLock.h>
#endif

#ifndef ESB_SIMPLE_PERFORMANCE_COUNTER_H
#include <ESBSimplePerformanceCounter.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ESB {

HistoricalPerformanceCounter::HistoricalPerformanceCounter(const char *name,
                                                           UInt16 windowSizeSec,
                                                           Allocator *allocator)
    : PerformanceCounter(),
      _windowSizeSec(windowSizeSec),
      _name(name),
      _list(),
      _lock(),
      _allocator(sizeof(SimplePerformanceCounter) * 60 * 5, allocator) {}

HistoricalPerformanceCounter::~HistoricalPerformanceCounter() {
  // Manually call the destructors of all the performance counters.   Their
  // memory will be cleaned up by the discard allocator's destructor.

  SimplePerformanceCounter *previous = 0;
  SimplePerformanceCounter *current =
      (SimplePerformanceCounter *)_list.getFirst();

  while (current) {
    previous = current;

    current = (SimplePerformanceCounter *)current->getNext();

    previous->~SimplePerformanceCounter();
  }
}

void HistoricalPerformanceCounter::addObservation(const Date &start,
                                                  const Date &stop) {
  SimplePerformanceCounter *counter = 0;

  {
    WriteScopeLock lock(_lock);

    counter = (SimplePerformanceCounter *)_list.getLast();

    if (!counter || counter->getWindowStop() < stop) {
      // We're in a new window.  Create a new perf counter and make it the new
      // head

      Date windowStart((stop.getSeconds() / _windowSizeSec) * _windowSizeSec,
                       0);
      Date windowStop(windowStart.getSeconds() + _windowSizeSec, 0);

      counter = new (&_allocator)
          SimplePerformanceCounter(_name, windowStart, windowStop);

      if (!counter) {
        ESB_LOG_WARNING(
            "Cannot allocate memory for new window, dropping observation");
        return;
      }

      _list.addLast(counter);
    }
  }

  counter->addObservation(start, stop);
}

void HistoricalPerformanceCounter::printSummary(FILE *file) const {
  ReadScopeLock lock(_lock);

  for (SimplePerformanceCounter *counter =
           (SimplePerformanceCounter *)_list.getFirst();
       counter; counter = (SimplePerformanceCounter *)counter->getNext()) {
    counter->printSummary(file);
  }
}

}  // namespace ESB
