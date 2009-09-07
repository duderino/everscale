/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
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

class AWSHttpServerCounters
{
public:
    AWSHttpServerCounters();

    virtual ~AWSHttpServerCounters();

    void printSummary();

    inline AWSPerformanceCounter *getSuccessfulTransactions()
    {
        return &_successfulTransactions;
    }

    inline AWSPerformanceCounter *getRequestHeaderErrors()
    {
        return &_requestHeaderErrors;
    }

    inline AWSPerformanceCounter *getRequestHeaderFailures()
    {
        return &_requestHeaderFailures;
    }

    inline AWSPerformanceCounter *getRequestHeaderTimeouts()
    {
        return &_requestHeaderTimeouts;
    }

    inline AWSPerformanceCounter *getRequestBodyErrors()
    {
        return &_requestBodyErrors;
    }

    inline AWSPerformanceCounter *getRequestBodyFailures()
    {
        return &_requestBodyFailures;
    }

    inline AWSPerformanceCounter *getRequestBodyTimeouts()
    {
        return &_requestBodyTimeouts;
    }

    inline AWSPerformanceCounter *getResponseHeaderErrors()
    {
        return &_responseHeaderErrors;
    }

    inline AWSPerformanceCounter *getResponseHeaderFailures()
    {
        return &_responseHeaderFailures;
    }

    inline AWSPerformanceCounter *getResponseHeaderTimeouts()
    {
        return &_responseHeaderTimeouts;
    }

    inline AWSPerformanceCounter *getResponseBodyErrors()
    {
        return &_responseBodyErrors;
    }

    inline AWSPerformanceCounter *getResponseBodyFailures()
    {
        return &_responseBodyFailures;
    }

    inline AWSPerformanceCounter *getResponseBodyTimeouts()
    {
        return &_responseBodyTimeouts;
    }

    inline ESFSharedCounter *getTotalConnections()
    {
        return &_totalConnections;
    }

    inline AWSAveragingCounter *getAverageTransactionsPerConnection()
    {
        return &_averageTransactionsPerConnection;
    }

private:
    // Disabled
    AWSHttpServerCounters(const AWSHttpServerCounters &counters);
    void operator=(const AWSHttpServerCounters &counters);

    AWSPerformanceCounter _successfulTransactions;

    AWSPerformanceCounter _requestHeaderErrors;
    AWSPerformanceCounter _requestHeaderFailures;
    AWSPerformanceCounter _requestHeaderTimeouts;
    AWSPerformanceCounter _requestBodyErrors;
    AWSPerformanceCounter _requestBodyFailures;
    AWSPerformanceCounter _requestBodyTimeouts;
    AWSPerformanceCounter _responseHeaderErrors;
    AWSPerformanceCounter _responseHeaderFailures;
    AWSPerformanceCounter _responseHeaderTimeouts;
    AWSPerformanceCounter _responseBodyErrors;
    AWSPerformanceCounter _responseBodyFailures;
    AWSPerformanceCounter _responseBodyTimeouts;

    ESFSharedCounter _totalConnections;
    AWSAveragingCounter _averageTransactionsPerConnection;
};

#endif
