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
                       UInt16 windowSizeSec, Allocator *allocator)
    : PerformanceCounter(),
      _currentWindows(0U),
      _maxWindows(maxWindows),
      _windowSizeSec(windowSizeSec),
      _name(name),
      _lock(),
      _list(),
      _allocator(allocator ? allocator : SystemAllocator::GetInstance()) {}

TimeSeries::~TimeSeries() {
  SimplePerformanceCounter *previous = 0;
  SimplePerformanceCounter *current =
      (SimplePerformanceCounter *)_list.getFirst();

  while (current) {
    previous = current;
    current = (SimplePerformanceCounter *)current->getNext();
    previous->~SimplePerformanceCounter();
    _allocator->deallocate(previous);
  }
}

void TimeSeries::addObservation(const Date &start, const Date &stop) {
  WriteScopeLock lock(_lock);
  SimplePerformanceCounter *counter =
      (SimplePerformanceCounter *)_list.getLast();

  if (!counter || counter->getWindowStop() < stop) {
    // We're in a new window.

    Date windowStart((stop.getSeconds() / _windowSizeSec) * _windowSizeSec, 0);
    Date windowStop(windowStart.getSeconds() + _windowSizeSec, 0);

    if (_currentWindows >= _maxWindows) {
      counter = (SimplePerformanceCounter *)_list.removeFirst();
      assert(counter);
      // reuse the oldest counter's memory for the new counter.
      counter = new (counter)
          SimplePerformanceCounter(_name, windowStart, windowStop);
      counter->~SimplePerformanceCounter();
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

  counter->addObservation(start, stop);
}

void TimeSeries::log(Logger &logger, Logger::Severity severity) const {
  ReadScopeLock lock(_lock);

  for (SimplePerformanceCounter *counter =
           (SimplePerformanceCounter *)_list.getFirst();
       counter; counter = (SimplePerformanceCounter *)counter->getNext()) {
    counter->log(logger, severity);
  }
}

}  // namespace ESB
