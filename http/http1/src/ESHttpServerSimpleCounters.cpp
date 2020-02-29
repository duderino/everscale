#ifndef ES_HTTP_SERVER_SIMPLE_COUNTERS_H
#include <ESHttpServerSimpleCounters.h>
#endif

namespace ES {

HttpServerSimpleCounters::HttpServerSimpleCounters()
    : _successfulTransactions("SERVER TRANS SUCCESS"),
      _requestHeaderErrors("SERVER TRANS REQUEST HEADER ERROR"),
      _requestHeaderFailures("SERVER TRANS REQUST HEADER FAILURE"),
      _requestHeaderTimeouts("SERVER TRANS REQUEST HEADER TIMEOUT"),
      _requestBodyErrors("SERVER TRANS REQUEST BODY ERROR"),
      _requestBodyFailures("SERVER TRANS REQUEST BODY FAILURE"),
      _requestBodyTimeouts("SERVER TRANS REQUEST BODY TIMEOUT"),
      _responseHeaderErrors("SERVER TRANS RESB::PONSE HEADER ERROR"),
      _responseHeaderFailures("SERVER TRANS RESB::PONSE HEADER FAILURE"),
      _responseHeaderTimeouts("SERVER TRANS RESB::PONSE HEADER TIMEOUT"),
      _responseBodyErrors("SERVER TRANS RESB::PONSE BODY ERROR"),
      _responseBodyFailures("SERVER TRANS RESB::PONSE BODY FAILURE"),
      _responseBodyTimeouts("SERVER TRANS RESB::PONSE BODY TIMEOUT"),
      _totalConnections(),
      _averageTransactionsPerConnection() {}

HttpServerSimpleCounters::~HttpServerSimpleCounters() {}

void HttpServerSimpleCounters::printSummary(FILE *file) const {
  fprintf(file, "SERVER CONNECTION ACCEPTS: %d\n", _totalConnections.get());
  fprintf(file, "SERVER AVG TRANS PER CONNECTION: %.2f\n",
          _averageTransactionsPerConnection.getValue());
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getSuccessfulTransactions() {
  return &_successfulTransactions;
}

const ESB::PerformanceCounter *
HttpServerSimpleCounters::getSuccessfulTransactions() const {
  return &_successfulTransactions;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestHeaderErrors() {
  return &_requestHeaderErrors;
}

const ESB::PerformanceCounter *
HttpServerSimpleCounters::getRequestHeaderErrors() const {
  return &_requestHeaderErrors;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestHeaderFailures() {
  return &_requestHeaderFailures;
}

const ESB::PerformanceCounter *
HttpServerSimpleCounters::getRequestHeaderFailures() const {
  return &_requestHeaderFailures;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestHeaderTimeouts() {
  return &_requestHeaderTimeouts;
}

const ESB::PerformanceCounter *
HttpServerSimpleCounters::getRequestHeaderTimeouts() const {
  return &_requestHeaderTimeouts;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestBodyErrors() {
  return &_requestBodyErrors;
}

const ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestBodyErrors()
    const {
  return &_requestBodyErrors;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestBodyFailures() {
  return &_requestBodyFailures;
}

const ESB::PerformanceCounter *
HttpServerSimpleCounters::getRequestBodyFailures() const {
  return &_requestBodyFailures;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestBodyTimeouts() {
  return &_requestBodyTimeouts;
}

const ESB::PerformanceCounter *
HttpServerSimpleCounters::getRequestBodyTimeouts() const {
  return &_requestBodyTimeouts;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseHeaderErrors() {
  return &_responseHeaderErrors;
}

const ESB::PerformanceCounter *
HttpServerSimpleCounters::getResponseHeaderErrors() const {
  return &_responseHeaderErrors;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseHeaderFailures() {
  return &_responseHeaderFailures;
}

const ESB::PerformanceCounter *
HttpServerSimpleCounters::getResponseHeaderFailures() const {
  return &_responseHeaderFailures;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseHeaderTimeouts() {
  return &_responseHeaderTimeouts;
}

const ESB::PerformanceCounter *
HttpServerSimpleCounters::getResponseHeaderTimeouts() const {
  return &_responseHeaderTimeouts;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseBodyErrors() {
  return &_responseBodyErrors;
}

const ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseBodyErrors()
    const {
  return &_responseBodyErrors;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseBodyFailures() {
  return &_responseBodyFailures;
}

const ESB::PerformanceCounter *
HttpServerSimpleCounters::getResponseBodyFailures() const {
  return &_responseBodyFailures;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseBodyTimeouts() {
  return &_responseBodyTimeouts;
}

const ESB::PerformanceCounter *
HttpServerSimpleCounters::getResponseBodyTimeouts() const {
  return &_responseBodyTimeouts;
}

ESB::SharedCounter *HttpServerSimpleCounters::getTotalConnections() {
  return &_totalConnections;
}

const ESB::SharedCounter *HttpServerSimpleCounters::getTotalConnections()
    const {
  return &_totalConnections;
}

ESB::AveragingCounter *
HttpServerSimpleCounters::getAverageTransactionsPerConnection() {
  return &_averageTransactionsPerConnection;
}

const ESB::AveragingCounter *
HttpServerSimpleCounters::getAverageTransactionsPerConnection() const {
  return &_averageTransactionsPerConnection;
}

}  // namespace ES
