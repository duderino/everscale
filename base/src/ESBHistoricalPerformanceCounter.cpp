#ifndef ESB_HISTORICAL_PERFORMANCE_COUNTER_H
#include <ESBHistoricalPerformanceCounter.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

#ifndef ESB_SIMPLE_PERFORMANCE_COUNTER_H
#include <ESBSimplePerformanceCounter.h>
#endif

namespace ESB {

HistoricalPerformanceCounter::HistoricalPerformanceCounter(const char *name,
                                                           time_t windowSizeSec,
                                                           Allocator *allocator,
                                                           Logger *logger)
    : PerformanceCounter(),
      _windowSizeSec(windowSizeSec),
      _name(name),
      _logger(logger),
      _list(),
      _lock(),
      _allocator(sizeof(SimplePerformanceCounter) * 1000, allocator) {}

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

void HistoricalPerformanceCounter::addObservation(const struct timeval *start) {
  struct timeval now;

  SimplePerformanceCounter::GetTime(&now);

  addObservation(start, &now);
}

void HistoricalPerformanceCounter::addObservation(const struct timeval *start,
                                                  const struct timeval *stop) {
  SimplePerformanceCounter *counter = 0;

  {
    WriteScopeLock lock(_lock);

    counter = (SimplePerformanceCounter *)_list.getLast();

    if (0 == counter || counter->getStopTime() < stop->tv_sec) {
      // We're in a new window.  Create a new perf counter and make it the new
      // head

      time_t startTime = (stop->tv_sec / _windowSizeSec) * _windowSizeSec;
      time_t stopTime = startTime + _windowSizeSec;

      counter = new (&_allocator)
          SimplePerformanceCounter(_name, startTime, stopTime);

      if (0 == counter) {
        if (_logger->isLoggable(Logger::Warning)) {
          _logger->log(
              Logger::Warning, __FILE__, __LINE__,
              "Cannot allocate memory for new window, dropping observation");
        }

        return;
      }

      _list.addLast(counter);
    }
  }

  counter->addObservation(start, stop);
}

void HistoricalPerformanceCounter::printSummary(FILE *file) const {
  WriteScopeLock lock(_lock);

  fprintf(file, "{ \"%s\": [", _name);

  for (SimplePerformanceCounter *counter =
           (SimplePerformanceCounter *)_list.getFirst();
       counter; counter = (SimplePerformanceCounter *)counter->getNext()) {
    fprintf(file, "\t");

    counter->printSummary(file);

    if (counter->getNext()) {
      fprintf(file, ",\n");
    } else {
      fprintf(file, "\n");
    }
  }

  fprintf(file, "]}");
}

}  // namespace ESB
