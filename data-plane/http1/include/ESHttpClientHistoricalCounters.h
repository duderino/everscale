#ifndef ES_HTTP_CLIENT_HISTORICAL_COUNTERS_H
#define ES_HTTP_CLIENT_HISTORICAL_COUNTERS_H

#ifndef ES_HTTP_CLIENT_COUNTERS_H
#include <ESHttpClientCounters.h>
#endif

#ifndef ESB_TIME_SERIES_H
#include <ESBTimeSeries.h>
#endif

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

namespace ES {

class HttpClientHistoricalCounters : public HttpClientCounters {
 public:
  /**
   * Constructor.
   *
   * @param windowSizeSec The counter's latency and throughput will be saved
   *   for each window of this many seconds.
   * @param allocator The counter will grab memory from this allocator
   *   for each new window it creates.
   */
  HttpClientHistoricalCounters(ESB::UInt16 maxWindows, ESB::UInt16 windowSizeSec, ESB::Allocator &allocator,
                               bool isProxy = false);

  virtual ~HttpClientHistoricalCounters();

  virtual void log(ESB::Logger &logger, ESB::Logger::Severity severity) const;

  virtual ESB::PerformanceCounter &successfulTransactions();
  virtual const ESB::PerformanceCounter &successfulTransactions() const;

  virtual ESB::PerformanceCounter &failedTransactions();
  virtual const ESB::PerformanceCounter &failedTransactions() const;

  virtual ESB::PerformanceCounter &requestBeginError();
  virtual const ESB::PerformanceCounter &requestBeginError() const;

  virtual ESB::PerformanceCounter &requestResolveError();
  virtual const ESB::PerformanceCounter &requestResolveError() const;

  virtual ESB::PerformanceCounter &requestConnectError();
  virtual const ESB::PerformanceCounter &requestConnectError() const;

  virtual ESB::PerformanceCounter &requestHeaderSendError();
  virtual const ESB::PerformanceCounter &requestHeaderSendError() const;

  virtual ESB::PerformanceCounter &requestBodySendError();
  virtual const ESB::PerformanceCounter &requestBodySendError() const;

  virtual ESB::PerformanceCounter &responseHeaderReceiveError();
  virtual const ESB::PerformanceCounter &responseHeaderReceiveError() const;

  virtual ESB::PerformanceCounter &responseBodyReceiveError();
  virtual const ESB::PerformanceCounter &responseBodyReceiveError() const;

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
  // All the time series counters will grow to their max window size
  // by allocating from this allocator.  Sharing an allocator across
  // all the counters will keep their windows close to each other.
  ESB::DiscardAllocator _allocator;
  ESB::TimeSeries _successfulTransactions;
  ESB::TimeSeries _failedTransactions;

  ESB::TimeSeries _requestBeginError;
  ESB::TimeSeries _requestResolveError;
  ESB::TimeSeries _requestConnectError;
  ESB::TimeSeries _requestHeaderSendError;
  ESB::TimeSeries _requestBodySendError;
  ESB::TimeSeries _responseHeaderReceiveError;
  ESB::TimeSeries _responseBodyReceiveError;

  ESB::TimeSeries _responseStatus1xx;

  ESB::TimeSeries _responseStatus200;
  ESB::TimeSeries _responseStatus201;
  ESB::TimeSeries _responseStatus202;
  ESB::TimeSeries _responseStatus204;
  ESB::TimeSeries _responseStatus2xx;

  ESB::TimeSeries _responseStatus304;
  ESB::TimeSeries _responseStatus3xx;

  ESB::TimeSeries _responseStatus400;
  ESB::TimeSeries _responseStatus401;
  ESB::TimeSeries _responseStatus403;
  ESB::TimeSeries _responseStatus404;
  ESB::TimeSeries _responseStatus410;
  ESB::TimeSeries _responseStatus4xx;

  ESB::TimeSeries _responseStatus500;
  ESB::TimeSeries _responseStatus502;
  ESB::TimeSeries _responseStatus503;
  ESB::TimeSeries _responseStatus504;
  ESB::TimeSeries _responseStatus5xx;

  ESB::TimeSeries _responseStatusOther;
  bool _isProxy;

  ESB_DEFAULT_FUNCS(HttpClientHistoricalCounters);
};

}  // namespace ES

#endif
