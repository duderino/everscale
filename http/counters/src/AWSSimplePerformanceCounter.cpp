/* Copyright (c) 2011 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_SIMPLE_PERFORMANCE_COUNTER_H
#include <AWSSimplePerformanceCounter.h>
#endif

#include <stdio.h>

AWSSimplePerformanceCounter::AWSSimplePerformanceCounter(const char *name)
    : AWSPerformanceCounter(),
      _startTime(0),
      _stopTime(0),
      _name(name),
      _avgLatencyMsec(0.0),
      _minLatencyMsec(-1.0),
      _maxLatencyMsec(0.0),
      _throughput(0UL),
      _lock() {}

AWSSimplePerformanceCounter::AWSSimplePerformanceCounter(const char *name,
                                                         time_t startTime,
                                                         time_t stopTime)
    : AWSPerformanceCounter(),
      _startTime(startTime),
      _stopTime(stopTime),
      _name(name),
      _avgLatencyMsec(0.0),
      _minLatencyMsec(-1.0),
      _maxLatencyMsec(0.0),
      _throughput(0UL),
      _lock() {}

AWSSimplePerformanceCounter::~AWSSimplePerformanceCounter() {}

void AWSSimplePerformanceCounter::addObservation(const struct timeval *start) {
  struct timeval now;

  gettimeofday(&now, 0);

  addObservation(start, &now);
}

void AWSSimplePerformanceCounter::addObservation(const struct timeval *start,
                                                 const struct timeval *stop) {
  double latencyMsec = (stop->tv_sec - start->tv_sec) * 1000.0;

  latencyMsec += stop->tv_usec / 1000.0;
  latencyMsec -= start->tv_usec / 1000.0;

  _lock.writeAcquire();

  ++_throughput;

  double throughput = _throughput;

  _avgLatencyMsec = (latencyMsec * (1.0 / throughput)) +
                    (_avgLatencyMsec * ((throughput - 1.0) / throughput));

  if (0 > _minLatencyMsec) {
    _minLatencyMsec = latencyMsec;
  } else if (_minLatencyMsec > latencyMsec) {
    _minLatencyMsec = latencyMsec;
  }

  if (latencyMsec > _maxLatencyMsec) {
    _maxLatencyMsec = latencyMsec;
  }

  _lock.writeRelease();
}

void AWSSimplePerformanceCounter::printSummary(FILE *file) const {
  _lock.writeAcquire();

  double avgLatencyMsec = _avgLatencyMsec;
  double minLatencyMsec = _minLatencyMsec;
  double maxLatencyMsec = _maxLatencyMsec;
  unsigned long throughput = _throughput;

  _lock.writeRelease();

  fprintf(
      file,
      "{\"name\": \"%s\", \"n\": %lu, \"avglatmillis\": %f, \"minlatmillis\": "
      "%f, \"maxlatmillis\": %f, \"start\": %lu, \"stop\": %lu}",
      _name, throughput, avgLatencyMsec, minLatencyMsec, maxLatencyMsec,
      _startTime, _stopTime);
}
