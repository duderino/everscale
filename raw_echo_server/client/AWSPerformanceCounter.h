/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_PERFORMANCE_COUNTER_H
#define AWS_PERFORMANCE_COUNTER_H

#include <pthread.h>
#include <sys/time.h>

class AWSPerformanceCounter {
 public:
  AWSPerformanceCounter(const char *name);

  virtual ~AWSPerformanceCounter();

  static void GetTime(struct timeval *now);

  void addObservation(struct timeval *start);

  inline const char *getName() { return _name; }

  inline unsigned long getThroughput() { return _throughput; }

  inline double getAverageLatencyMsec() { return _avgLatencyMsec; }

  inline double getMinLatencyMsec() {
    return 0 > _minLatencyMsec ? 0 : _minLatencyMsec;
  }

  inline double getMaxLatencyMsec() { return _maxLatencyMsec; }

  void printSummary();

 private:
  // Disabled
  AWSPerformanceCounter(const AWSPerformanceCounter &counter);
  AWSPerformanceCounter *operator=(const AWSPerformanceCounter &counter);

  const char *_name;
  pthread_mutex_t _lock;
  double _avgLatencyMsec;
  double _minLatencyMsec;
  double _maxLatencyMsec;
  unsigned long _throughput;
};

#endif
