/* Copyright (c) 2011 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_CLIENT_SIMPLE_COUNTERS_H
#include <AWSHttpClientSimpleCounters.h>
#endif

AWSHttpClientSimpleCounters::AWSHttpClientSimpleCounters()
    : _successes("CLIENT TRANS SUCCESS"), _failures("CLIENT TRANS FAILURE") {}

AWSHttpClientSimpleCounters::~AWSHttpClientSimpleCounters() {}

void AWSHttpClientSimpleCounters::printSummary(FILE *file) const {
  _successes.printSummary(file);
  _failures.printSummary(file);
}

AWSPerformanceCounter *AWSHttpClientSimpleCounters::getSuccesses() {
  return &_successes;
}

const AWSPerformanceCounter *AWSHttpClientSimpleCounters::getSuccesses() const {
  return &_successes;
}

AWSPerformanceCounter *AWSHttpClientSimpleCounters::getFailures() {
  return &_failures;
}

const AWSPerformanceCounter *AWSHttpClientSimpleCounters::getFailures() const {
  return &_failures;
}
