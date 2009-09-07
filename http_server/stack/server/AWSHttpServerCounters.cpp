/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_SERVER_COUNTERS_H
#include <AWSHttpServerCounters.h>
#endif

#include <stdio.h>

AWSHttpServerCounters::AWSHttpServerCounters() :
    _successfulTransactions("SERVER TRANS SUCCESS"),
    _requestHeaderErrors("SERVER TRANS REQUEST HEADER ERROR"),
    _requestHeaderFailures("SERVER TRANS REQUST HEADER FAILURE"),
    _requestHeaderTimeouts("SERVER TRANS REQUEST HEADER TIMEOUT"),
    _requestBodyErrors("SERVER TRANS REQUEST BODY ERROR"),
    _requestBodyFailures("SERVER TRANS REQUEST BODY FAILURE"),
    _requestBodyTimeouts("SERVER TRANS REQUEST BODY TIMEOUT"),
    _responseHeaderErrors("SERVER TRANS RESPONSE HEADER ERROR"),
    _responseHeaderFailures("SERVER TRANS RESPONSE HEADER FAILURE"),
    _responseHeaderTimeouts("SERVER TRANS RESPONSE HEADER TIMEOUT"),
    _responseBodyErrors("SERVER TRANS RESPONSE BODY ERROR"),
    _responseBodyFailures("SERVER TRANS RESPONSE BODY FAILURE"),
    _responseBodyTimeouts("SERVER TRANS RESPONSE BODY TIMEOUT"),
    _totalConnections(),
    _averageTransactionsPerConnection()
{
}

AWSHttpServerCounters::~AWSHttpServerCounters()
{
}

void AWSHttpServerCounters::printSummary()
{
    /*_successfulTransactions.printSummary();
    _requestHeaderErrors.printSummary();
    _requestHeaderFailures.printSummary();
    _requestHeaderTimeouts.printSummary();
    _requestBodyErrors.printSummary();
    _requestBodyFailures.printSummary();
    _requestBodyTimeouts.printSummary();
    _responseHeaderErrors.printSummary();
    _responseHeaderFailures.printSummary();
    _responseHeaderTimeouts.printSummary();
    _responseBodyErrors.printSummary();
    _responseBodyFailures.printSummary();
    _responseBodyTimeouts.printSummary();*/

    fprintf(stderr, "Accepted Connections: %d\n", _totalConnections.get());
    fprintf(stderr, "Requests Per Connection:\n");
    fprintf(stderr, "\tavg: %f\n", _averageTransactionsPerConnection.getValue());
    fprintf(stderr, "\tn: %f\n", _averageTransactionsPerConnection.getObservations());
}


