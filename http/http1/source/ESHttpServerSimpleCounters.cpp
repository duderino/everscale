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
      _responseHeaderErrors("SERVER TRANS RESPONSE HEADER ERROR"),
      _responseHeaderFailures("SERVER TRANS RESPONSE HEADER FAILURE"),
      _responseHeaderTimeouts("SERVER TRANS RESPONSE HEADER TIMEOUT"),
      _responseBodyErrors("SERVER TRANS RESPONSE BODY ERROR"),
      _responseBodyFailures("SERVER TRANS RESPONSE BODY FAILURE"),
      _responseBodyTimeouts("SERVER TRANS RESPONSE BODY TIMEOUT"),
      _totalConnections(),
      _averageTransactionsPerConnection() {}

HttpServerSimpleCounters::~HttpServerSimpleCounters() {}

void HttpServerSimpleCounters::log(ESB::Logger &logger, ESB::Logger::Severity severity) const {
  /*_successfulTransactions.log(logger, severity);
  _requestHeaderErrors.log(logger, severity);
  _requestHeaderFailures.log(logger, severity);
  _requestHeaderTimeouts.log(logger, severity);
  _requestBodyErrors.log(logger, severity);
  _requestBodyFailures.log(logger, severity);
  _requestBodyTimeouts.log(logger, severity);
  _responseHeaderErrors.log(logger, severity);
  _responseHeaderFailures.log(logger, severity);
  _responseHeaderTimeouts.log(logger, severity);
  _responseBodyErrors.log(logger, severity);
  _responseBodyFailures.log(logger, severity);
  _responseHeaderTimeouts.log(logger, severity);*/
  ESB_LOG(logger, severity, "SERVER CONNECTION ACCEPTS: %d", _totalConnections.get());
  _averageTransactionsPerConnection.log(logger, severity, "SERVER AVG TRANS PER CONNECTION");
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getSuccessfulTransactions() { return &_successfulTransactions; }

const ESB::PerformanceCounter *HttpServerSimpleCounters::getSuccessfulTransactions() const {
  return &_successfulTransactions;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestHeaderErrors() { return &_requestHeaderErrors; }

const ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestHeaderErrors() const {
  return &_requestHeaderErrors;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestHeaderFailures() { return &_requestHeaderFailures; }

const ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestHeaderFailures() const {
  return &_requestHeaderFailures;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestHeaderTimeouts() { return &_requestHeaderTimeouts; }

const ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestHeaderTimeouts() const {
  return &_requestHeaderTimeouts;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestBodyErrors() { return &_requestBodyErrors; }

const ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestBodyErrors() const { return &_requestBodyErrors; }

ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestBodyFailures() { return &_requestBodyFailures; }

const ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestBodyFailures() const {
  return &_requestBodyFailures;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestBodyTimeouts() { return &_requestBodyTimeouts; }

const ESB::PerformanceCounter *HttpServerSimpleCounters::getRequestBodyTimeouts() const {
  return &_requestBodyTimeouts;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseHeaderErrors() { return &_responseHeaderErrors; }

const ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseHeaderErrors() const {
  return &_responseHeaderErrors;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseHeaderFailures() { return &_responseHeaderFailures; }

const ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseHeaderFailures() const {
  return &_responseHeaderFailures;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseHeaderTimeouts() { return &_responseHeaderTimeouts; }

const ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseHeaderTimeouts() const {
  return &_responseHeaderTimeouts;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseBodyErrors() { return &_responseBodyErrors; }

const ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseBodyErrors() const { return &_responseBodyErrors; }

ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseBodyFailures() { return &_responseBodyFailures; }

const ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseBodyFailures() const {
  return &_responseBodyFailures;
}

ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseBodyTimeouts() { return &_responseBodyTimeouts; }

const ESB::PerformanceCounter *HttpServerSimpleCounters::getResponseBodyTimeouts() const {
  return &_responseBodyTimeouts;
}

ESB::SharedInt *HttpServerSimpleCounters::getTotalConnections() { return &_totalConnections; }

const ESB::SharedInt *HttpServerSimpleCounters::getTotalConnections() const { return &_totalConnections; }

ESB::SharedAveragingCounter *HttpServerSimpleCounters::getAverageTransactionsPerConnection() {
  return &_averageTransactionsPerConnection;
}

const ESB::SharedAveragingCounter *HttpServerSimpleCounters::getAverageTransactionsPerConnection() const {
  return &_averageTransactionsPerConnection;
}

}  // namespace ES
