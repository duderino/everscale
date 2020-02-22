/* Copyright (c) 2011 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_CLIENT_COUNTERS_H
#define AWS_HTTP_CLIENT_COUNTERS_H

#ifndef AWS_PERFORMANCE_COUNTER_H
#include <AWSPerformanceCounter.h>
#endif

class AWSHttpClientCounters {
 public:
  AWSHttpClientCounters();

  virtual ~AWSHttpClientCounters();

  virtual void printSummary(FILE *file) const = 0;

  virtual AWSPerformanceCounter *getSuccesses() = 0;

  virtual const AWSPerformanceCounter *getSuccesses() const = 0;

  virtual AWSPerformanceCounter *getFailures() = 0;

  virtual const AWSPerformanceCounter *getFailures() const = 0;

 private:
  // Disabled
  AWSHttpClientCounters(const AWSHttpClientCounters &counters);
  void operator=(const AWSHttpClientCounters &counters);
};

#endif
