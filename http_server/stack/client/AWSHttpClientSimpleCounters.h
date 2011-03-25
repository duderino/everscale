/* Copyright (c) 2011 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_CLIENT_SIMPLE_COUNTERS_H
#define AWS_HTTP_CLIENT_SIMPLE_COUNTERS_H

#ifndef AWS_HTTP_CLIENT_COUNTERS_H
#include <AWSHttpClientCounters.h>
#endif

#ifndef AWS_SIMPLE_PERFORMANCE_COUNTER_H
#include <AWSSimplePerformanceCounter.h>
#endif

class AWSHttpClientSimpleCounters: public AWSHttpClientCounters {
public:
    AWSHttpClientSimpleCounters();

    virtual ~AWSHttpClientSimpleCounters();

    virtual void printSummary(FILE *file) const;

    virtual AWSPerformanceCounter *getSuccesses();

    virtual const AWSPerformanceCounter *getSuccesses() const;

    virtual AWSPerformanceCounter *getFailures();

    virtual const AWSPerformanceCounter *getFailures() const;

private:
    // Disabled
    AWSHttpClientSimpleCounters(const AWSHttpClientSimpleCounters &counters);
    void operator=(const AWSHttpClientSimpleCounters &counters);

    AWSSimplePerformanceCounter _successes;
    AWSSimplePerformanceCounter _failures;
};

#endif
