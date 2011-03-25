/* Copyright (c) 2011 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_PERFORMANCE_COUNTER_H
#include <AWSPerformanceCounter.h>
#endif

#include <sys/time.h>

AWSPerformanceCounter::AWSPerformanceCounter() : ESFEmbeddedListElement() {
}

AWSPerformanceCounter::~AWSPerformanceCounter() {
}

void AWSPerformanceCounter::GetTime(struct timeval *now) {
    gettimeofday(now, 0);
}

ESFCleanupHandler *AWSPerformanceCounter::getCleanupHandler() {
    return 0;
}
