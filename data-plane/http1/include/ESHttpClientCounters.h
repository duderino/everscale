#ifndef ES_HTTP_CLIENT_COUNTERS_H
#define ES_HTTP_CLIENT_COUNTERS_H

#ifndef ESB_PERFORMANCE_COUNTER_H
#include <ESBPerformanceCounter.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ES {

class HttpClientCounters {
 public:
  HttpClientCounters();

  virtual ~HttpClientCounters();

  void incrementStatusCounter(int statusCode, const ESB::Date &start, const ESB::Date &stop);

  virtual void log(ESB::Logger &logger, ESB::Logger::Severity severity) const = 0;

  virtual ESB::PerformanceCounter &successfulTransactions() = 0;
  virtual const ESB::PerformanceCounter &successfulTransactions() const = 0;

  virtual ESB::PerformanceCounter &failedTransactions() = 0;
  virtual const ESB::PerformanceCounter &failedTransactions() const = 0;

  virtual ESB::PerformanceCounter &requestBeginError() = 0;
  virtual const ESB::PerformanceCounter &requestBeginError() const = 0;

  virtual ESB::PerformanceCounter &requestResolveError() = 0;
  virtual const ESB::PerformanceCounter &requestResolveError() const = 0;

  virtual ESB::PerformanceCounter &requestConnectError() = 0;
  virtual const ESB::PerformanceCounter &requestConnectError() const = 0;

  virtual ESB::PerformanceCounter &requestHeaderSendError() = 0;
  virtual const ESB::PerformanceCounter &requestHeaderSendError() const = 0;

  virtual ESB::PerformanceCounter &requestBodySendError() = 0;
  virtual const ESB::PerformanceCounter &requestBodySendError() const = 0;

  virtual ESB::PerformanceCounter &responseHeaderReceiveError() = 0;
  virtual const ESB::PerformanceCounter &responseHeaderReceiveError() const = 0;

  virtual ESB::PerformanceCounter &responseBodyReceiveError() = 0;
  virtual const ESB::PerformanceCounter &responseBodyReceiveError() const = 0;

  virtual ESB::PerformanceCounter &responseStatus1xx() = 0;
  virtual const ESB::PerformanceCounter &responseStatus1xx() const = 0;

  virtual ESB::PerformanceCounter &responseStatus200() = 0;
  virtual const ESB::PerformanceCounter &responseStatus200() const = 0;

  virtual ESB::PerformanceCounter &responseStatus201() = 0;
  virtual const ESB::PerformanceCounter &responseStatus201() const = 0;

  virtual ESB::PerformanceCounter &responseStatus202() = 0;
  virtual const ESB::PerformanceCounter &responseStatus202() const = 0;

  virtual ESB::PerformanceCounter &responseStatus204() = 0;
  virtual const ESB::PerformanceCounter &responseStatus204() const = 0;

  virtual ESB::PerformanceCounter &responseStatus2xx() = 0;
  virtual const ESB::PerformanceCounter &responseStatus2xx() const = 0;

  virtual ESB::PerformanceCounter &responseStatus304() = 0;
  virtual const ESB::PerformanceCounter &responseStatus304() const = 0;

  virtual ESB::PerformanceCounter &responseStatus3xx() = 0;
  virtual const ESB::PerformanceCounter &responseStatus3xx() const = 0;

  virtual ESB::PerformanceCounter &responseStatus400() = 0;
  virtual const ESB::PerformanceCounter &responseStatus400() const = 0;

  virtual ESB::PerformanceCounter &responseStatus401() = 0;
  virtual const ESB::PerformanceCounter &responseStatus401() const = 0;

  virtual ESB::PerformanceCounter &responseStatus403() = 0;
  virtual const ESB::PerformanceCounter &responseStatus403() const = 0;

  virtual ESB::PerformanceCounter &responseStatus404() = 0;
  virtual const ESB::PerformanceCounter &responseStatus404() const = 0;

  virtual ESB::PerformanceCounter &responseStatus410() = 0;
  virtual const ESB::PerformanceCounter &responseStatus410() const = 0;

  virtual ESB::PerformanceCounter &responseStatus4xx() = 0;
  virtual const ESB::PerformanceCounter &responseStatus4xx() const = 0;

  virtual ESB::PerformanceCounter &responseStatus500() = 0;
  virtual const ESB::PerformanceCounter &responseStatus500() const = 0;

  virtual ESB::PerformanceCounter &responseStatus502() = 0;
  virtual const ESB::PerformanceCounter &responseStatus502() const = 0;

  virtual ESB::PerformanceCounter &responseStatus503() = 0;
  virtual const ESB::PerformanceCounter &responseStatus503() const = 0;

  virtual ESB::PerformanceCounter &responseStatus504() = 0;
  virtual const ESB::PerformanceCounter &responseStatus504() const = 0;

  virtual ESB::PerformanceCounter &responseStatus5xx() = 0;
  virtual const ESB::PerformanceCounter &responseStatus5xx() const = 0;

  virtual ESB::PerformanceCounter &responseStatusOther() = 0;
  virtual const ESB::PerformanceCounter &responseStatusOther() const = 0;

  ESB_DISABLE_AUTO_COPY(HttpClientCounters);
};

}  // namespace ES

#endif
