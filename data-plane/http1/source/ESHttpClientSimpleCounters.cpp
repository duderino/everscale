#ifndef ES_HTTP_CLIENT_SIMPLE_COUNTERS_H
#include <ESHttpClientSimpleCounters.h>
#endif

namespace ES {

HttpClientSimpleCounters::HttpClientSimpleCounters(bool isProxy)
    : _successfulTransactions(isProxy ? "PROXY "
                                      : "CLIENT "
                                        "CLIENT TRANS SUCCESS"),
      _failedTransactions(isProxy ? "PROXY "
                                  : "CLIENT "
                                    "CLIENT TRANS FAILURE"),
      _requestBeginError(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS REQUEST BEGIN ERROR"),
      _requestResolveError(isProxy ? "PROXY "
                                   : "CLIENT "
                                     "CLIENT TRANS REQUEST RESOLVE ERROR"),
      _requestConnectError(isProxy ? "PROXY "
                                   : "CLIENT "
                                     "CLIENT TRANS REQUEST CONNECT ERROR"),
      _requestHeaderSendError(isProxy ? "PROXY "
                                      : "CLIENT "
                                        "CLIENT TRANS REQUEST HEADER SEND ERROR"),
      _requestBodySendError(isProxy ? "PROXY "
                                    : "CLIENT "
                                      "CLIENT TRANS REQUEST BODY SEND ERROR"),
      _responseHeaderReceiveError(isProxy ? "PROXY "
                                          : "CLIENT "
                                            "CLIENT TRANS RESPONSE HEADER RECEIVE ERROR"),
      _responseBodyReceiveError(isProxy ? "PROXY "
                                        : "CLIENT "
                                          "CLIENT TRANS RESPONSE BODY RECEIVE ERROR"),
      _responseStatus1xx(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 1XX"),
      _responseStatus200(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 200"),
      _responseStatus201(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 201"),
      _responseStatus202(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 202"),
      _responseStatus204(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 204"),
      _responseStatus2xx(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 2XX"),
      _responseStatus304(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 304"),
      _responseStatus3xx(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 3XX"),
      _responseStatus400(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 400"),
      _responseStatus401(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 401"),
      _responseStatus403(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 403"),
      _responseStatus404(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 404"),
      _responseStatus410(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 410"),
      _responseStatus4xx(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 4XX"),
      _responseStatus500(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 500"),
      _responseStatus502(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 502"),
      _responseStatus503(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 503"),
      _responseStatus504(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 504"),
      _responseStatus5xx(isProxy ? "PROXY "
                                 : "CLIENT "
                                   "CLIENT TRANS RESPONSE STATUS 5XX"),
      _responseStatusOther(isProxy ? "PROXY "
                                   : "CLIENT "
                                     "CLIENT TRANS RESPONSE STATUS OTHER"),
      _isProxy(isProxy) {}

HttpClientSimpleCounters::~HttpClientSimpleCounters() {}

void HttpClientSimpleCounters::log(ESB::Logger &logger, ESB::Logger::Severity severity) const {
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

ESB::PerformanceCounter &HttpClientSimpleCounters::successfulTransactions() { return _successfulTransactions; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::successfulTransactions() const {
  return _successfulTransactions;
}

ESB::PerformanceCounter &HttpClientSimpleCounters::failedTransactions() { return _failedTransactions; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::failedTransactions() const { return _failedTransactions; }

ESB::PerformanceCounter &HttpClientSimpleCounters::requestBeginError() { return _requestBeginError; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::requestBeginError() const { return _requestBeginError; }

ESB::PerformanceCounter &HttpClientSimpleCounters::requestResolveError() { return _requestResolveError; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::requestResolveError() const { return _requestResolveError; }

ESB::PerformanceCounter &HttpClientSimpleCounters::requestConnectError() { return _requestConnectError; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::requestConnectError() const { return _requestConnectError; }

ESB::PerformanceCounter &HttpClientSimpleCounters::requestHeaderSendError() { return _requestHeaderSendError; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::requestHeaderSendError() const {
  return _requestHeaderSendError;
}

ESB::PerformanceCounter &HttpClientSimpleCounters::requestBodySendError() { return _requestBodySendError; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::requestBodySendError() const { return _requestBodySendError; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseHeaderReceiveError() { return _responseHeaderReceiveError; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseHeaderReceiveError() const {
  return _responseHeaderReceiveError;
}

ESB::PerformanceCounter &HttpClientSimpleCounters::responseBodyReceiveError() { return _responseBodyReceiveError; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseBodyReceiveError() const {
  return _responseBodyReceiveError;
}

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus1xx() { return _responseStatus1xx; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus1xx() const { return _responseStatus1xx; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus200() { return _responseStatus200; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus200() const { return _responseStatus200; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus201() { return _responseStatus201; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus201() const { return _responseStatus201; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus202() { return _responseStatus202; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus202() const { return _responseStatus202; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus204() { return _responseStatus204; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus204() const { return _responseStatus204; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus2xx() { return _responseStatus2xx; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus2xx() const { return _responseStatus2xx; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus304() { return _responseStatus304; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus304() const { return _responseStatus304; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus3xx() { return _responseStatus3xx; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus3xx() const { return _responseStatus3xx; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus400() { return _responseStatus400; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus400() const { return _responseStatus400; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus401() { return _responseStatus401; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus401() const { return _responseStatus401; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus403() { return _responseStatus403; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus403() const { return _responseStatus403; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus404() { return _responseStatus404; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus404() const { return _responseStatus404; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus410() { return _responseStatus410; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus410() const { return _responseStatus410; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus4xx() { return _responseStatus4xx; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus4xx() const { return _responseStatus4xx; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus500() { return _responseStatus500; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus500() const { return _responseStatus500; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus502() { return _responseStatus502; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus502() const { return _responseStatus502; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus503() { return _responseStatus503; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus503() const { return _responseStatus503; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus504() { return _responseStatus504; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus504() const { return _responseStatus504; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus5xx() { return _responseStatus5xx; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatus5xx() const { return _responseStatus5xx; }

ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatusOther() { return _responseStatusOther; }

const ESB::PerformanceCounter &HttpClientSimpleCounters::responseStatusOther() const { return _responseStatusOther; }

}  // namespace ES
