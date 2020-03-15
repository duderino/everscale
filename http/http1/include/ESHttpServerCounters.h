#ifndef ES_HTTP_SERVER_COUNTERS_H
#define ES_HTTP_SERVER_COUNTERS_H

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

#ifndef ESB_AVERAGING_COUNTER_H
#include <ESBAveragingCounter.h>
#endif

#ifndef ESB_PERFORMANCE_COUNTER_H
#include <ESBPerformanceCounter.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ES {
class HttpServerCounters {
 public:
  HttpServerCounters();

  virtual ~HttpServerCounters();

  virtual void log(ESB::Logger &logger,
                   ESB::Logger::Severity severity) const = 0;

  virtual ESB::PerformanceCounter *getSuccessfulTransactions() = 0;

  virtual const ESB::PerformanceCounter *getSuccessfulTransactions() const = 0;

  virtual ESB::PerformanceCounter *getRequestHeaderErrors() = 0;

  virtual const ESB::PerformanceCounter *getRequestHeaderErrors() const = 0;

  virtual ESB::PerformanceCounter *getRequestHeaderFailures() = 0;

  virtual const ESB::PerformanceCounter *getRequestHeaderFailures() const = 0;

  virtual ESB::PerformanceCounter *getRequestHeaderTimeouts() = 0;

  virtual const ESB::PerformanceCounter *getRequestHeaderTimeouts() const = 0;

  virtual ESB::PerformanceCounter *getRequestBodyErrors() = 0;

  virtual const ESB::PerformanceCounter *getRequestBodyErrors() const = 0;

  virtual ESB::PerformanceCounter *getRequestBodyFailures() = 0;

  virtual const ESB::PerformanceCounter *getRequestBodyFailures() const = 0;

  virtual ESB::PerformanceCounter *getRequestBodyTimeouts() = 0;

  virtual const ESB::PerformanceCounter *getRequestBodyTimeouts() const = 0;

  virtual ESB::PerformanceCounter *getResponseHeaderErrors() = 0;

  virtual const ESB::PerformanceCounter *getResponseHeaderErrors() const = 0;

  virtual ESB::PerformanceCounter *getResponseHeaderFailures() = 0;

  virtual const ESB::PerformanceCounter *getResponseHeaderFailures() const = 0;

  virtual ESB::PerformanceCounter *getResponseHeaderTimeouts() = 0;

  virtual const ESB::PerformanceCounter *getResponseHeaderTimeouts() const = 0;

  virtual ESB::PerformanceCounter *getResponseBodyErrors() = 0;

  virtual const ESB::PerformanceCounter *getResponseBodyErrors() const = 0;

  virtual ESB::PerformanceCounter *getResponseBodyFailures() = 0;

  virtual const ESB::PerformanceCounter *getResponseBodyFailures() const = 0;

  virtual ESB::PerformanceCounter *getResponseBodyTimeouts() = 0;

  virtual const ESB::PerformanceCounter *getResponseBodyTimeouts() const = 0;

  virtual ESB::SharedInt *getTotalConnections() = 0;

  virtual const ESB::SharedInt *getTotalConnections() const = 0;

  virtual ESB::AveragingCounter *getAverageTransactionsPerConnection() = 0;

  virtual const ESB::AveragingCounter *getAverageTransactionsPerConnection()
      const = 0;

 private:
  // Disabled
  HttpServerCounters(const HttpServerCounters &counters);
  void operator=(const HttpServerCounters &counters);
};
}  // namespace ES

#endif
