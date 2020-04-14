#ifndef ESB_TIME_SERIES_H
#include <ESBTimeSeries.h>
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

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ESB {

TimeSeries::TimeSeries(const char *name, UInt16 maxWindows,
                       UInt16 windowSizeSec, Allocator &allocator)
    : PerformanceCounter(),
      _currentWindows(0U),
      _maxWindows(maxWindows),
      _windowSizeSec(windowSizeSec),
      _name(name),
      _lock(),
      _list(),
      _allocator(allocator) {}

TimeSeries::~TimeSeries() {
  SimplePerformanceCounter *previous = 0;
  SimplePerformanceCounter *current = (SimplePerformanceCounter *)_list.first();

  while (current) {
    previous = current;
    current = (SimplePerformanceCounter *)current->next();
    previous->~SimplePerformanceCounter();
    _allocator.deallocate(previous);
  }
}

void TimeSeries::record(const Date &start, const Date &stop) {
  WriteScopeLock lock(_lock);
  SimplePerformanceCounter *counter = (SimplePerformanceCounter *)_list.last();

  if (!counter || counter->windowStop() < stop) {
    // We're in a new window.

    Date windowStart((stop.seconds() / _windowSizeSec) * _windowSizeSec, 0);
    Date windowStop(windowStart.seconds() + _windowSizeSec, 0);

    if (_currentWindows >= _maxWindows) {
      counter = (SimplePerformanceCounter *)_list.removeFirst();
      assert(counter);
      // reuse the oldest counter's memory for the new counter.
      counter->~SimplePerformanceCounter();
      counter = new (counter)
          SimplePerformanceCounter(_name, windowStart, windowStop);
    } else {
      counter = new (_allocator)
          SimplePerformanceCounter(_name, windowStart, windowStop);
      if (!counter) {
        ESB_LOG_WARNING("Cannot allocate memory for new window");
        return;
      }
      ++_currentWindows;
    }

    _list.addLast(counter);
  }

  counter->record(start, stop);
}

UInt32 TimeSeries::queries() const {
  ReadScopeLock lock(_lock);

  UInt32 queries = 0;

  for (const SimplePerformanceCounter *counter =
           (const SimplePerformanceCounter *)_list.first();
       counter; counter = (const SimplePerformanceCounter *)counter->next()) {
    queries += counter->queries();
  }

  return queries;
}

void TimeSeries::log(Logger &logger, Logger::Severity severity) const {
  ReadScopeLock lock(_lock);

  for (SimplePerformanceCounter *counter =
           (SimplePerformanceCounter *)_list.first();
       counter; counter = (SimplePerformanceCounter *)counter->next()) {
    counter->log(logger, severity);
  }
}

}  // namespace ESB
