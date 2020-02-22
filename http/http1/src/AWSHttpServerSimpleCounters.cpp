/* Copyright (c) 2011 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_SERVER_SIMPLE_COUNTERS_H
#include <AWSHttpServerSimpleCounters.h>
#endif

#include <stdio.h>

AWSHttpServerSimpleCounters::AWSHttpServerSimpleCounters()
    : _successfulTransactions("SERVER TRANS SUCCESS"),
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
      _averageTransactionsPerConnection() {}

AWSHttpServerSimpleCounters::~AWSHttpServerSimpleCounters() {}

void AWSHttpServerSimpleCounters::printSummary(FILE *file) const {
  fprintf(file, "Accepted Connections: %d\n", _totalConnections.get());
  fprintf(file, "Requests Per Connection:\n");
  fprintf(file, "\tavg: %f\n", _averageTransactionsPerConnection.getValue());
  fprintf(file, "\tn: %f\n",
          _averageTransactionsPerConnection.getObservations());
}

AWSPerformanceCounter *
AWSHttpServerSimpleCounters::getSuccessfulTransactions() {
  return &_successfulTransactions;
}

const AWSPerformanceCounter *
AWSHttpServerSimpleCounters::getSuccessfulTransactions() const {
  return &_successfulTransactions;
}

AWSPerformanceCounter *AWSHttpServerSimpleCounters::getRequestHeaderErrors() {
  return &_requestHeaderErrors;
}

const AWSPerformanceCounter *
AWSHttpServerSimpleCounters::getRequestHeaderErrors() const {
  return &_requestHeaderErrors;
}

AWSPerformanceCounter *AWSHttpServerSimpleCounters::getRequestHeaderFailures() {
  return &_requestHeaderFailures;
}

const AWSPerformanceCounter *
AWSHttpServerSimpleCounters::getRequestHeaderFailures() const {
  return &_requestHeaderFailures;
}

AWSPerformanceCounter *AWSHttpServerSimpleCounters::getRequestHeaderTimeouts() {
  return &_requestHeaderTimeouts;
}

const AWSPerformanceCounter *
AWSHttpServerSimpleCounters::getRequestHeaderTimeouts() const {
  return &_requestHeaderTimeouts;
}

AWSPerformanceCounter *AWSHttpServerSimpleCounters::getRequestBodyErrors() {
  return &_requestBodyErrors;
}

const AWSPerformanceCounter *AWSHttpServerSimpleCounters::getRequestBodyErrors()
    const {
  return &_requestBodyErrors;
}

AWSPerformanceCounter *AWSHttpServerSimpleCounters::getRequestBodyFailures() {
  return &_requestBodyFailures;
}

const AWSPerformanceCounter *
AWSHttpServerSimpleCounters::getRequestBodyFailures() const {
  return &_requestBodyFailures;
}

AWSPerformanceCounter *AWSHttpServerSimpleCounters::getRequestBodyTimeouts() {
  return &_requestBodyTimeouts;
}

const AWSPerformanceCounter *
AWSHttpServerSimpleCounters::getRequestBodyTimeouts() const {
  return &_requestBodyTimeouts;
}

AWSPerformanceCounter *AWSHttpServerSimpleCounters::getResponseHeaderErrors() {
  return &_responseHeaderErrors;
}

const AWSPerformanceCounter *
AWSHttpServerSimpleCounters::getResponseHeaderErrors() const {
  return &_responseHeaderErrors;
}

AWSPerformanceCounter *
AWSHttpServerSimpleCounters::getResponseHeaderFailures() {
  return &_responseHeaderFailures;
}

const AWSPerformanceCounter *
AWSHttpServerSimpleCounters::getResponseHeaderFailures() const {
  return &_responseHeaderFailures;
}

AWSPerformanceCounter *
AWSHttpServerSimpleCounters::getResponseHeaderTimeouts() {
  return &_responseHeaderTimeouts;
}

const AWSPerformanceCounter *
AWSHttpServerSimpleCounters::getResponseHeaderTimeouts() const {
  return &_responseHeaderTimeouts;
}

AWSPerformanceCounter *AWSHttpServerSimpleCounters::getResponseBodyErrors() {
  return &_responseBodyErrors;
}

const AWSPerformanceCounter *
AWSHttpServerSimpleCounters::getResponseBodyErrors() const {
  return &_responseBodyErrors;
}

AWSPerformanceCounter *AWSHttpServerSimpleCounters::getResponseBodyFailures() {
  return &_responseBodyFailures;
}

const AWSPerformanceCounter *
AWSHttpServerSimpleCounters::getResponseBodyFailures() const {
  return &_responseBodyFailures;
}

AWSPerformanceCounter *AWSHttpServerSimpleCounters::getResponseBodyTimeouts() {
  return &_responseBodyTimeouts;
}

const AWSPerformanceCounter *
AWSHttpServerSimpleCounters::getResponseBodyTimeouts() const {
  return &_responseBodyTimeouts;
}

ESFSharedCounter *AWSHttpServerSimpleCounters::getTotalConnections() {
  return &_totalConnections;
}

const ESFSharedCounter *AWSHttpServerSimpleCounters::getTotalConnections()
    const {
  return &_totalConnections;
}

AWSAveragingCounter *
AWSHttpServerSimpleCounters::getAverageTransactionsPerConnection() {
  return &_averageTransactionsPerConnection;
}

const AWSAveragingCounter *
AWSHttpServerSimpleCounters::getAverageTransactionsPerConnection() const {
  return &_averageTransactionsPerConnection;
}
