#ifndef ES_HTTP_CLIENT_HISTORICAL_COUNTERS_H
#include <ESHttpClientHistoricalCounters.h>
#endif

namespace ES {

HttpClientHistoricalCounters::HttpClientHistoricalCounters(ESB::UInt16 maxWindows, ESB::UInt16 windowSizeSec,
                                                           ESB::Allocator &allocator, bool isProxy)
    : _allocator(4096 - ESB::DiscardAllocator::SizeofChunk(sizeof(ESB::Word)), sizeof(ESB::Word), 1, allocator),
  _successfulTransactions(isProxy ? "PROXY "
                                      : "CLIENT "
                                        "CLIENT TRANS SUCCESS",
                              maxWindows, windowSizeSec, _allocator),
      _failedTransactions(isProxy ? "PROXY "
                                  : "CLIENT "
                                    "CLIENT TRANS FAILURE",
                          maxWindows, windowSizeSec, _allocator),
      _requestBeginError(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS REQUEST BEGIN ERROR",
                         maxWindows, windowSizeSec, _allocator),
      _requestResolveError(isProxy ? "PROXY "
                                   : "CLIENT "
                                     "CLIENT TRANS REQUEST RESOLVE ERROR",
                           maxWindows, windowSizeSec, _allocator),
      _requestConnectError(isProxy ? "PROXY "
                                   : "CLIENT "
                                     "CLIENT TRANS REQUEST CONNECT ERROR",
                           maxWindows, windowSizeSec, _allocator),
      _requestHeaderSendError(isProxy ? "PROXY "
                                      : "CLIENT "
                                        "CLIENT TRANS REQUEST HEADER SEND ERROR",
                              maxWindows, windowSizeSec, _allocator),
      _requestBodySendError(isProxy ? "PROXY "
                                    : "CLIENT "
                                      "CLIENT TRANS REQUEST BODY SEND ERROR",
                            maxWindows, windowSizeSec, _allocator),
      _responseHeaderReceiveError(isProxy ? "PROXY "
                                          : "CLIENT "
                                            "CLIENT TRANS RESPONSE HEADER RECEIVE ERROR",
                                  maxWindows, windowSizeSec, _allocator),
      _responseBodyReceiveError(isProxy ? "PROXY "
                                        : "CLIENT "
                                          "CLIENT TRANS RESPONSE BODY RECEIVE ERROR",
                                maxWindows, windowSizeSec, _allocator),
      _responseStatus1xx(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 1XX",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus200(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 200",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus201(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 201",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus202(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 202",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus204(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 204",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus2xx(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 2XX",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus304(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 304",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus3xx(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 3XX",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus400(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 400",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus401(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 401",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus403(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 403",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus404(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 404",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus410(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 410",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus4xx(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 4XX",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus500(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 500",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus502(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 502",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus503(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 503",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus504(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 504",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatus5xx(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 5XX",
                         maxWindows, windowSizeSec, _allocator),
      _responseStatusOther(isProxy ? "PROXY "
                                   : "CLIENT "
                                     "CLIENT TRANS RESPONSE STATUS OTHER",
                           maxWindows, windowSizeSec, _allocator),
      _isProxy(isProxy) {}

HttpClientHistoricalCounters::~HttpClientHistoricalCounters() {}

void HttpClientHistoricalCounters::log(ESB::Logger &logger, ESB::Logger::Severity severity) const {
  _successfulTransactions.log(logger, severity);
  _failedTransactions.log(logger, severity);
  _requestBeginError.log(logger, severity);
  _requestResolveError.log(logger, severity);
  _requestConnectError.log(logger, severity);
  _requestHeaderSendError.log(logger, severity);
  _requestBodySendError.log(logger, severity);
  _responseHeaderReceiveError.log(logger, severity);
  _responseBodyReceiveError.log(logger, severity);
  _responseStatus1xx.log(logger, severity);
  _responseStatus200.log(logger, severity);
  _responseStatus201.log(logger, severity);
  _responseStatus202.log(logger, severity);
  _responseStatus204.log(logger, severity);
  _responseStatus2xx.log(logger, severity);
  _responseStatus304.log(logger, severity);
  _responseStatus3xx.log(logger, severity);
  _responseStatus400.log(logger, severity);
  _responseStatus401.log(logger, severity);
  _responseStatus403.log(logger, severity);
  _responseStatus404.log(logger, severity);
  _responseStatus410.log(logger, severity);
  _responseStatus4xx.log(logger, severity);
  _responseStatus500.log(logger, severity);
  _responseStatus502.log(logger, severity);
  _responseStatus503.log(logger, severity);
  _responseStatus504.log(logger, severity);
  _responseStatus5xx.log(logger, severity);
  _responseStatusOther.log(logger, severity);
}

ESB::PerformanceCounter &HttpClientHistoricalCounters::successfulTransactions() { return _successfulTransactions; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::successfulTransactions() const {
  return _successfulTransactions;
}

ESB::PerformanceCounter &HttpClientHistoricalCounters::failedTransactions() { return _failedTransactions; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::failedTransactions() const { return _failedTransactions; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::requestBeginError() { return _requestBeginError; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::requestBeginError() const { return _requestBeginError; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::requestResolveError() { return _requestResolveError; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::requestResolveError() const {
  return _requestResolveError;
}

ESB::PerformanceCounter &HttpClientHistoricalCounters::requestConnectError() { return _requestConnectError; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::requestConnectError() const {
  return _requestConnectError;
}

ESB::PerformanceCounter &HttpClientHistoricalCounters::requestHeaderSendError() { return _requestHeaderSendError; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::requestHeaderSendError() const {
  return _requestHeaderSendError;
}

ESB::PerformanceCounter &HttpClientHistoricalCounters::requestBodySendError() { return _requestBodySendError; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::requestBodySendError() const {
  return _requestBodySendError;
}

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseHeaderReceiveError() {
  return _responseHeaderReceiveError;
}

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseHeaderReceiveError() const {
  return _responseHeaderReceiveError;
}

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseBodyReceiveError() { return _responseBodyReceiveError; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseBodyReceiveError() const {
  return _responseBodyReceiveError;
}

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus1xx() { return _responseStatus1xx; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus1xx() const { return _responseStatus1xx; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus200() { return _responseStatus200; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus200() const { return _responseStatus200; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus201() { return _responseStatus201; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus201() const { return _responseStatus201; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus202() { return _responseStatus202; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus202() const { return _responseStatus202; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus204() { return _responseStatus204; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus204() const { return _responseStatus204; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus2xx() { return _responseStatus2xx; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus2xx() const { return _responseStatus2xx; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus304() { return _responseStatus304; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus304() const { return _responseStatus304; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus3xx() { return _responseStatus3xx; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus3xx() const { return _responseStatus3xx; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus400() { return _responseStatus400; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus400() const { return _responseStatus400; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus401() { return _responseStatus401; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus401() const { return _responseStatus401; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus403() { return _responseStatus403; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus403() const { return _responseStatus403; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus404() { return _responseStatus404; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus404() const { return _responseStatus404; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus410() { return _responseStatus410; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus410() const { return _responseStatus410; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus4xx() { return _responseStatus4xx; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus4xx() const { return _responseStatus4xx; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus500() { return _responseStatus500; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus500() const { return _responseStatus500; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus502() { return _responseStatus502; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus502() const { return _responseStatus502; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus503() { return _responseStatus503; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus503() const { return _responseStatus503; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus504() { return _responseStatus504; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus504() const { return _responseStatus504; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus5xx() { return _responseStatus5xx; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatus5xx() const { return _responseStatus5xx; }

ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatusOther() { return _responseStatusOther; }

const ESB::PerformanceCounter &HttpClientHistoricalCounters::responseStatusOther() const {
  return _responseStatusOther;
}

}  // namespace ES
