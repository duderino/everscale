/* Copyright (c) 2011 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_CLIENT_HISTORICAL_COUNTERS_H
#include <AWSHttpClientHistoricalCounters.h>
#endif

AWSHttpClientHistoricalCounters::AWSHttpClientHistoricalCounters(time_t windowSizeSec, ESFAllocator *allocator, ESFLogger *logger) :
    _successes("CLIENT TRANS SUCCESS", windowSizeSec, allocator, logger), _failures("CLIENT TRANS FAILURE", windowSizeSec, allocator, logger) {
}

AWSHttpClientHistoricalCounters::~AWSHttpClientHistoricalCounters() {
}

void AWSHttpClientHistoricalCounters::printSummary(FILE *file) const {
    fprintf(file, "{");

    _successes.printSummary(file);

    fprintf(file, ",\n");

    _failures.printSummary(file);

    fprintf(file, "}");
}

AWSPerformanceCounter *AWSHttpClientHistoricalCounters::getSuccesses() {
    return &_successes;
}

const AWSPerformanceCounter *AWSHttpClientHistoricalCounters::getSuccesses() const {
    return &_successes;
}

AWSPerformanceCounter *AWSHttpClientHistoricalCounters::getFailures() {
    return &_failures;
}

const AWSPerformanceCounter *AWSHttpClientHistoricalCounters::getFailures() const {
    return &_failures;
}

