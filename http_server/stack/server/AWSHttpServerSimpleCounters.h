/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_SERVER_SIMPLE_COUNTERS_H
#define AWS_HTTP_SERVER_SIMPLE_COUNTERS_H

#ifndef ESF_SHARED_COUNTER_H
#include <ESFSharedCounter.h>
#endif

#ifndef AWS_AVERAGING_COUNTER_H
#include <AWSAveragingCounter.h>
#endif

#ifndef AWS_HTTP_SERVER_COUNTERS_H
#include <AWSHttpServerCounters.h>
#endif

#ifndef AWS_SIMPLE_PERFORMANCE_COUNTER_H
#include <AWSSimplePerformanceCounter.h>
#endif

class AWSHttpServerSimpleCounters: public AWSHttpServerCounters {
public:
    AWSHttpServerSimpleCounters();

    virtual ~AWSHttpServerSimpleCounters();

    virtual void printSummary(FILE *file) const;

    virtual AWSPerformanceCounter *getSuccessfulTransactions();

    virtual const AWSPerformanceCounter *getSuccessfulTransactions() const;

    virtual AWSPerformanceCounter *getRequestHeaderErrors();

    virtual const AWSPerformanceCounter *getRequestHeaderErrors() const;

    virtual AWSPerformanceCounter *getRequestHeaderFailures();

    virtual const AWSPerformanceCounter *getRequestHeaderFailures() const;

    virtual AWSPerformanceCounter *getRequestHeaderTimeouts();

    virtual const AWSPerformanceCounter *getRequestHeaderTimeouts() const;

    virtual AWSPerformanceCounter *getRequestBodyErrors();

    virtual const AWSPerformanceCounter *getRequestBodyErrors() const;

    virtual AWSPerformanceCounter *getRequestBodyFailures();

    virtual const AWSPerformanceCounter *getRequestBodyFailures() const;

    virtual AWSPerformanceCounter *getRequestBodyTimeouts();

    virtual const AWSPerformanceCounter *getRequestBodyTimeouts() const;

    virtual AWSPerformanceCounter *getResponseHeaderErrors();

    virtual const AWSPerformanceCounter *getResponseHeaderErrors() const;

    virtual AWSPerformanceCounter *getResponseHeaderFailures();

    virtual const AWSPerformanceCounter *getResponseHeaderFailures() const;

    virtual AWSPerformanceCounter *getResponseHeaderTimeouts();

    virtual const AWSPerformanceCounter *getResponseHeaderTimeouts() const;

    virtual AWSPerformanceCounter *getResponseBodyErrors();

    virtual const AWSPerformanceCounter *getResponseBodyErrors() const;

    virtual AWSPerformanceCounter *getResponseBodyFailures();

    virtual const AWSPerformanceCounter *getResponseBodyFailures() const;

    virtual AWSPerformanceCounter *getResponseBodyTimeouts();

    virtual const AWSPerformanceCounter *getResponseBodyTimeouts() const;

    virtual ESFSharedCounter *getTotalConnections();

    virtual const ESFSharedCounter *getTotalConnections() const;

    virtual AWSAveragingCounter *getAverageTransactionsPerConnection();

    virtual const AWSAveragingCounter *getAverageTransactionsPerConnection() const;

private:
    // Disabled
    AWSHttpServerSimpleCounters(const AWSHttpServerSimpleCounters &counters);
    void operator=(const AWSHttpServerSimpleCounters &counters);

    AWSSimplePerformanceCounter _successfulTransactions;

    AWSSimplePerformanceCounter _requestHeaderErrors;
    AWSSimplePerformanceCounter _requestHeaderFailures;
    AWSSimplePerformanceCounter _requestHeaderTimeouts;
    AWSSimplePerformanceCounter _requestBodyErrors;
    AWSSimplePerformanceCounter _requestBodyFailures;
    AWSSimplePerformanceCounter _requestBodyTimeouts;
    AWSSimplePerformanceCounter _responseHeaderErrors;
    AWSSimplePerformanceCounter _responseHeaderFailures;
    AWSSimplePerformanceCounter _responseHeaderTimeouts;
    AWSSimplePerformanceCounter _responseBodyErrors;
    AWSSimplePerformanceCounter _responseBodyFailures;
    AWSSimplePerformanceCounter _responseBodyTimeouts;

    ESFSharedCounter _totalConnections;
    AWSAveragingCounter _averageTransactionsPerConnection;
};

#endif
