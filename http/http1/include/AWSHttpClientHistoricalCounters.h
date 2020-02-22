/* Copyright (c) 2011 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_CLIENT_HISTORICAL_COUNTERS_H
#define AWS_HTTP_CLIENT_HISTORICAL_COUNTERS_H

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

#ifndef AWS_HTTP_CLIENT_COUNTERS_H
#include <AWSHttpClientCounters.h>
#endif

#ifndef AWS_HISTORICAL_PERFORMANCE_COUNTER_H
#include <AWSHistoricalPerformanceCounter.h>
#endif

class AWSHttpClientHistoricalCounters : public AWSHttpClientCounters {
 public:
  /**
   * Constructor.
   *
   * @param windowSizeSec The counter's latency and throughput will be saved
   *   for each window of this many seconds.
   * @param allocator The counter will grab memory from this allocator
   *   for each new window it creates.
   */
  AWSHttpClientHistoricalCounters(time_t windowSizeSec, ESFAllocator *allocator,
                                  ESFLogger *logger);

  virtual ~AWSHttpClientHistoricalCounters();

  virtual void printSummary(FILE *file) const;

  virtual AWSPerformanceCounter *getSuccesses();

  virtual const AWSPerformanceCounter *getSuccesses() const;

  virtual AWSPerformanceCounter *getFailures();

  virtual const AWSPerformanceCounter *getFailures() const;

 private:
  // Disabled
  AWSHttpClientHistoricalCounters(
      const AWSHttpClientHistoricalCounters &counters);
  void operator=(const AWSHttpClientHistoricalCounters &counters);

  AWSHistoricalPerformanceCounter _successes;
  AWSHistoricalPerformanceCounter _failures;
};

#endif
