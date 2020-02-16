/* Copyright (c) 2011 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_PERFORMANCE_COUNTER_H
#define AWS_PERFORMANCE_COUNTER_H

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

#ifndef ESF_EMBEDDED_LIST_ELEMENT_H
#include <ESFEmbeddedListElement.h>
#endif

#include <sys/time.h>

class AWSPerformanceCounter : public ESFEmbeddedListElement {
 public:
  AWSPerformanceCounter();

  virtual ~AWSPerformanceCounter();

  static void GetTime(struct timeval *now);

  virtual void addObservation(const struct timeval *start) = 0;

  virtual void addObservation(const struct timeval *start,
                              const struct timeval *stop) = 0;

  virtual void printSummary(FILE *file) const = 0;

  virtual ESFCleanupHandler *getCleanupHandler();

 private:
  // Disabled
  AWSPerformanceCounter(const AWSPerformanceCounter &counter);
  AWSPerformanceCounter *operator=(const AWSPerformanceCounter &counter);
};

#endif
