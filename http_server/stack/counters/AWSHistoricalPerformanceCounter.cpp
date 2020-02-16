/* Copyright (c) 2011 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HISTORICAL_PERFORMANCE_COUNTER_H
#include <AWSHistoricalPerformanceCounter.h>
#endif

#ifndef ESF_WRITE_SCOPE_LOCK_H
#include <ESFWriteScopeLock.h>
#endif

AWSHistoricalPerformanceCounter::AWSHistoricalPerformanceCounter(
    const char *name, time_t windowSizeSec, ESFAllocator *allocator,
    ESFLogger *logger)
    : AWSPerformanceCounter(),
      _windowSizeSec(windowSizeSec),
      _name(name),
      _logger(logger),
      _list(),
      _lock(),
      _allocator(sizeof(AWSSimplePerformanceCounter) * 1000, allocator) {}

AWSHistoricalPerformanceCounter::~AWSHistoricalPerformanceCounter() {
  // Manually call the destructors of all the performance counters.   Their
  // memory will be cleaned up by the discard allocator's destructor.

  AWSSimplePerformanceCounter *previous = 0;
  AWSSimplePerformanceCounter *current =
      (AWSSimplePerformanceCounter *)_list.getFirst();

  while (current) {
    previous = current;

    current = (AWSSimplePerformanceCounter *)current->getNext();

    previous->~AWSSimplePerformanceCounter();
  }
}

void AWSHistoricalPerformanceCounter::addObservation(
    const struct timeval *start) {
  struct timeval now;

  AWSSimplePerformanceCounter::GetTime(&now);

  addObservation(start, &now);
}

void AWSHistoricalPerformanceCounter::addObservation(
    const struct timeval *start, const struct timeval *stop) {
  AWSSimplePerformanceCounter *counter = 0;

  {
    ESFWriteScopeLock lock(_lock);

    counter = (AWSSimplePerformanceCounter *)_list.getLast();

    if (0 == counter || counter->getStopTime() < stop->tv_sec) {
      // We're in a new window.  Create a new perf counter and make it the new
      // head

      time_t startTime = (stop->tv_sec / _windowSizeSec) * _windowSizeSec;
      time_t stopTime = startTime + _windowSizeSec;

      counter = new (&_allocator)
          AWSSimplePerformanceCounter(_name, startTime, stopTime);

      if (0 == counter) {
        if (_logger->isLoggable(ESFLogger::Warning)) {
          _logger->log(
              ESFLogger::Warning, __FILE__, __LINE__,
              "Cannot allocate memory for new window, dropping observation");
        }

        return;
      }

      _list.addLast(counter);
    }
  }

  counter->addObservation(start, stop);
}

void AWSHistoricalPerformanceCounter::printSummary(FILE *file) const {
  ESFWriteScopeLock lock(_lock);

  fprintf(file, "{ \"%s\": [", _name);

  for (AWSSimplePerformanceCounter *counter =
           (AWSSimplePerformanceCounter *)_list.getFirst();
       counter; counter = (AWSSimplePerformanceCounter *)counter->getNext()) {
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
