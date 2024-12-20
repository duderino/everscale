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
  HttpServerSimpleCounters(bool isProxy = false);

  virtual ~HttpServerSimpleCounters();

  virtual void log(ESB::Logger &logger, ESB::Logger::Severity severity) const;

  virtual ESB::PerformanceCounter &successfulTransactions();
  virtual const ESB::PerformanceCounter &successfulTransactions() const;

  virtual ESB::PerformanceCounter &failedTransactions();
  virtual const ESB::PerformanceCounter &failedTransactions() const;

  virtual ESB::PerformanceCounter &requestHeaderBeginError();
  virtual const ESB::PerformanceCounter &requestHeaderBeginError() const;

  virtual ESB::PerformanceCounter &requestHeaderReceiveError();
  virtual const ESB::PerformanceCounter &requestHeaderReceiveError() const;

  virtual ESB::PerformanceCounter &requestBodyReceiveError();
  virtual const ESB::PerformanceCounter &requestBodyReceiveError() const;

  virtual ESB::PerformanceCounter &requestBodySizeError();
  virtual const ESB::PerformanceCounter &requestBodySizeError() const;

  virtual ESB::PerformanceCounter &responseHeaderSendError();
  virtual const ESB::PerformanceCounter &responseHeaderSendError() const;

  virtual ESB::PerformanceCounter &responseBodySendError();
  virtual const ESB::PerformanceCounter &responseBodySendError() const;

  virtual ESB::PerformanceCounter &responseStatus1xx();
  virtual const ESB::PerformanceCounter &responseStatus1xx() const;

  virtual ESB::PerformanceCounter &responseStatus200();
  virtual const ESB::PerformanceCounter &responseStatus200() const;

  virtual ESB::PerformanceCounter &responseStatus201();
  virtual const ESB::PerformanceCounter &responseStatus201() const;

  virtual ESB::PerformanceCounter &responseStatus202();
  virtual const ESB::PerformanceCounter &responseStatus202() const;

  virtual ESB::PerformanceCounter &responseStatus204();
  virtual const ESB::PerformanceCounter &responseStatus204() const;

  virtual ESB::PerformanceCounter &responseStatus2xx();
  virtual const ESB::PerformanceCounter &responseStatus2xx() const;

  virtual ESB::PerformanceCounter &responseStatus304();
  virtual const ESB::PerformanceCounter &responseStatus304() const;

  virtual ESB::PerformanceCounter &responseStatus3xx();
  virtual const ESB::PerformanceCounter &responseStatus3xx() const;

  virtual ESB::PerformanceCounter &responseStatus400();
  virtual const ESB::PerformanceCounter &responseStatus400() const;

  virtual ESB::PerformanceCounter &responseStatus401();
  virtual const ESB::PerformanceCounter &responseStatus401() const;

  virtual ESB::PerformanceCounter &responseStatus403();
  virtual const ESB::PerformanceCounter &responseStatus403() const;

  virtual ESB::PerformanceCounter &responseStatus404();
  virtual const ESB::PerformanceCounter &responseStatus404() const;

  virtual ESB::PerformanceCounter &responseStatus410();
  virtual const ESB::PerformanceCounter &responseStatus410() const;

  virtual ESB::PerformanceCounter &responseStatus4xx();
  virtual const ESB::PerformanceCounter &responseStatus4xx() const;

  virtual ESB::PerformanceCounter &responseStatus500();
  virtual const ESB::PerformanceCounter &responseStatus500() const;

  virtual ESB::PerformanceCounter &responseStatus502();
  virtual const ESB::PerformanceCounter &responseStatus502() const;

  virtual ESB::PerformanceCounter &responseStatus503();
  virtual const ESB::PerformanceCounter &responseStatus503() const;

  virtual ESB::PerformanceCounter &responseStatus504();
  virtual const ESB::PerformanceCounter &responseStatus504() const;

  virtual ESB::PerformanceCounter &responseStatus5xx();
  virtual const ESB::PerformanceCounter &responseStatus5xx() const;

  virtual ESB::PerformanceCounter &responseStatusOther();
  virtual const ESB::PerformanceCounter &responseStatusOther() const;

 private:
  ESB::SimplePerformanceCounter _successfulTransactions;
  ESB::SimplePerformanceCounter _failedTransactions;

  ESB::SimplePerformanceCounter _requestHeaderBeginError;
  ESB::SimplePerformanceCounter _requestHeaderReceiveError;
  ESB::SimplePerformanceCounter _requestBodyReceiveError;
  ESB::SimplePerformanceCounter _requestBodySizeError;
  ESB::SimplePerformanceCounter _responseHeaderSendError;
  ESB::SimplePerformanceCounter _responseBodySendError;

  ESB::SimplePerformanceCounter _responseStatus1xx;

  ESB::SimplePerformanceCounter _responseStatus200;
  ESB::SimplePerformanceCounter _responseStatus201;
  ESB::SimplePerformanceCounter _responseStatus202;
  ESB::SimplePerformanceCounter _responseStatus204;
  ESB::SimplePerformanceCounter _responseStatus2xx;

  ESB::SimplePerformanceCounter _responseStatus304;
  ESB::SimplePerformanceCounter _responseStatus3xx;

  ESB::SimplePerformanceCounter _responseStatus400;
  ESB::SimplePerformanceCounter _responseStatus401;
  ESB::SimplePerformanceCounter _responseStatus403;
  ESB::SimplePerformanceCounter _responseStatus404;
  ESB::SimplePerformanceCounter _responseStatus410;
  ESB::SimplePerformanceCounter _responseStatus4xx;

  ESB::SimplePerformanceCounter _responseStatus500;
  ESB::SimplePerformanceCounter _responseStatus502;
  ESB::SimplePerformanceCounter _responseStatus503;
  ESB::SimplePerformanceCounter _responseStatus504;
  ESB::SimplePerformanceCounter _responseStatus5xx;

  ESB::SimplePerformanceCounter _responseStatusOther;

  bool _isProxy;

  ESB_DEFAULT_FUNCS(HttpServerSimpleCounters);
};
}  // namespace ES

#endif
