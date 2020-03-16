#ifndef ES_HTTP_SERVER_SIMPLE_COUNTERS_H
#define ES_HTTP_SERVER_SIMPLE_COUNTERS_H

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

#ifndef ESB_SHARED_AVERAGING_COUNTER_H
#include <ESBSharedAveragingCounter.h>
#endif

#ifndef ES_HTTP_SERVER_COUNTERS_H
#include <ESHttpServerCounters.h>
#endif

#ifndef ES_SIMPLE_PERFORMANCE_COUNTER_H
#include <ESBSimplePerformanceCounter.h>
#endif

namespace ES {

class HttpServerSimpleCounters : public HttpServerCounters {
 public:
  HttpServerSimpleCounters();

  virtual ~HttpServerSimpleCounters();

  virtual void log(ESB::Logger &logger, ESB::Logger::Severity severity) const;

  virtual ESB::PerformanceCounter *getSuccessfulTransactions();

  virtual const ESB::PerformanceCounter *getSuccessfulTransactions() const;

  virtual ESB::PerformanceCounter *getRequestHeaderErrors();

  virtual const ESB::PerformanceCounter *getRequestHeaderErrors() const;

  virtual ESB::PerformanceCounter *getRequestHeaderFailures();

  virtual const ESB::PerformanceCounter *getRequestHeaderFailures() const;

  virtual ESB::PerformanceCounter *getRequestHeaderTimeouts();

  virtual const ESB::PerformanceCounter *getRequestHeaderTimeouts() const;

  virtual ESB::PerformanceCounter *getRequestBodyErrors();

  virtual const ESB::PerformanceCounter *getRequestBodyErrors() const;

  virtual ESB::PerformanceCounter *getRequestBodyFailures();

  virtual const ESB::PerformanceCounter *getRequestBodyFailures() const;

  virtual ESB::PerformanceCounter *getRequestBodyTimeouts();

  virtual const ESB::PerformanceCounter *getRequestBodyTimeouts() const;

  virtual ESB::PerformanceCounter *getResponseHeaderErrors();

  virtual const ESB::PerformanceCounter *getResponseHeaderErrors() const;

  virtual ESB::PerformanceCounter *getResponseHeaderFailures();

  virtual const ESB::PerformanceCounter *getResponseHeaderFailures() const;

  virtual ESB::PerformanceCounter *getResponseHeaderTimeouts();

  virtual const ESB::PerformanceCounter *getResponseHeaderTimeouts() const;

  virtual ESB::PerformanceCounter *getResponseBodyErrors();

  virtual const ESB::PerformanceCounter *getResponseBodyErrors() const;

  virtual ESB::PerformanceCounter *getResponseBodyFailures();

  virtual const ESB::PerformanceCounter *getResponseBodyFailures() const;

  virtual ESB::PerformanceCounter *getResponseBodyTimeouts();

  virtual const ESB::PerformanceCounter *getResponseBodyTimeouts() const;

  virtual ESB::SharedInt *getTotalConnections();

  virtual const ESB::SharedInt *getTotalConnections() const;

  virtual ESB::SharedAveragingCounter *getAverageTransactionsPerConnection();

  virtual const ESB::SharedAveragingCounter *
  getAverageTransactionsPerConnection() const;

 private:
  // Disabled
  HttpServerSimpleCounters(const HttpServerSimpleCounters &counters);
  void operator=(const HttpServerSimpleCounters &counters);

  ESB::SimplePerformanceCounter _successfulTransactions;

  ESB::SimplePerformanceCounter _requestHeaderErrors;
  ESB::SimplePerformanceCounter _requestHeaderFailures;
  ESB::SimplePerformanceCounter _requestHeaderTimeouts;
  ESB::SimplePerformanceCounter _requestBodyErrors;
  ESB::SimplePerformanceCounter _requestBodyFailures;
  ESB::SimplePerformanceCounter _requestBodyTimeouts;
  ESB::SimplePerformanceCounter _responseHeaderErrors;
  ESB::SimplePerformanceCounter _responseHeaderFailures;
  ESB::SimplePerformanceCounter _responseHeaderTimeouts;
  ESB::SimplePerformanceCounter _responseBodyErrors;
  ESB::SimplePerformanceCounter _responseBodyFailures;
  ESB::SimplePerformanceCounter _responseBodyTimeouts;

  ESB::SharedInt _totalConnections;
  ESB::SharedAveragingCounter _averageTransactionsPerConnection;
};

}  // namespace ES

#endif
