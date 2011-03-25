/* Copyright (c) 2011 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_SERVER_COUNTERS_H
#define AWS_HTTP_SERVER_COUNTERS_H

#ifndef ESF_SHARED_COUNTER_H
#include <ESFSharedCounter.h>
#endif

#ifndef AWS_AVERAGING_COUNTER_H
#include <AWSAveragingCounter.h>
#endif

#ifndef AWS_PERFORMANCE_COUNTER_H
#include <AWSPerformanceCounter.h>
#endif

class AWSHttpServerCounters {
public:
    AWSHttpServerCounters();

    virtual ~AWSHttpServerCounters();

    virtual void printSummary(FILE *file) const = 0;

    virtual AWSPerformanceCounter *getSuccessfulTransactions() = 0;

    virtual const AWSPerformanceCounter *getSuccessfulTransactions() const = 0;

    virtual AWSPerformanceCounter *getRequestHeaderErrors() = 0;

    virtual const AWSPerformanceCounter *getRequestHeaderErrors() const = 0;

    virtual AWSPerformanceCounter *getRequestHeaderFailures() = 0;

    virtual const AWSPerformanceCounter *getRequestHeaderFailures() const = 0;

    virtual AWSPerformanceCounter *getRequestHeaderTimeouts() = 0;

    virtual const AWSPerformanceCounter *getRequestHeaderTimeouts() const = 0;

    virtual AWSPerformanceCounter *getRequestBodyErrors() = 0;

    virtual const AWSPerformanceCounter *getRequestBodyErrors() const = 0;

    virtual AWSPerformanceCounter *getRequestBodyFailures() = 0;

    virtual const AWSPerformanceCounter *getRequestBodyFailures() const = 0;

    virtual AWSPerformanceCounter *getRequestBodyTimeouts() = 0;

    virtual const AWSPerformanceCounter *getRequestBodyTimeouts() const = 0;

    virtual AWSPerformanceCounter *getResponseHeaderErrors() = 0;

    virtual const AWSPerformanceCounter *getResponseHeaderErrors() const = 0;

    virtual AWSPerformanceCounter *getResponseHeaderFailures() = 0;

    virtual const AWSPerformanceCounter *getResponseHeaderFailures() const = 0;

    virtual AWSPerformanceCounter *getResponseHeaderTimeouts() = 0;

    virtual const AWSPerformanceCounter *getResponseHeaderTimeouts() const = 0;

    virtual AWSPerformanceCounter *getResponseBodyErrors() = 0;

    virtual const AWSPerformanceCounter *getResponseBodyErrors() const = 0;

    virtual AWSPerformanceCounter *getResponseBodyFailures() = 0;

    virtual const AWSPerformanceCounter *getResponseBodyFailures() const = 0;

    virtual AWSPerformanceCounter *getResponseBodyTimeouts() = 0;

    virtual const AWSPerformanceCounter *getResponseBodyTimeouts() const = 0;

    virtual ESFSharedCounter *getTotalConnections() = 0;

    virtual const ESFSharedCounter *getTotalConnections() const = 0;

    virtual AWSAveragingCounter *getAverageTransactionsPerConnection() = 0;

    virtual const AWSAveragingCounter *getAverageTransactionsPerConnection() const = 0;

private:
    // Disabled
    AWSHttpServerCounters(const AWSHttpServerCounters &counters);
    void operator=(const AWSHttpServerCounters &counters);
};

#endif
