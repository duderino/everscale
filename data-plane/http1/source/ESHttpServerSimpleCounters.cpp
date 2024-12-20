#ifndef ES_HTTP_SERVER_SIMPLE_COUNTERS_H
#include <ESHttpServerSimpleCounters.h>
#endif

namespace ES {

HttpServerSimpleCounters::HttpServerSimpleCounters(bool isProxy)
    : _successfulTransactions(isProxy ? "PROXY "
                                      : "ORIGIN "
                                        "SERVER TRANS SUCCESS"),
      _failedTransactions(isProxy ? "PROXY "
                                  : "ORIGIN "
                                    "SERVER TRANS FAILURE"),
      _requestHeaderBeginError(isProxy ? "PROXY "
                                       : "ORIGIN "
                                         "SERVER TRANS REQUEST HEADER BEGIN ERROR"),
      _requestHeaderReceiveError(isProxy ? "PROXY "
                                         : "ORIGIN "
                                           "SERVER TRANS REQUEST HEADER RECEIVE ERROR"),
      _requestBodyReceiveError(isProxy ? "PROXY "
                                       : "ORIGIN "
                                         "SERVER TRANS REQUEST BODY RECEIVE ERROR"),
      _requestBodySizeError(isProxy ? "PROXY "
                                    : "ORIGIN "
                                      "SERVER TRANS REQUEST BODY SIZE ERROR"),
      _responseHeaderSendError(isProxy ? "PROXY "
                                       : "ORIGIN "
                                         "SERVER TRANS RESPONSE HEADER SEND ERROR"),
      _responseBodySendError(isProxy ? "PROXY "
                                     : "ORIGIN "
                                       "SERVER TRANS RESPONSE BODY SEND ERROR"),
      _responseStatus1xx(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 1XX"),
      _responseStatus200(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 200"),
      _responseStatus201(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 201"),
      _responseStatus202(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 202"),
      _responseStatus204(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 204"),
      _responseStatus2xx(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 2XX"),
      _responseStatus304(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 304"),
      _responseStatus3xx(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 3XX"),
      _responseStatus400(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 400"),
      _responseStatus401(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 401"),
      _responseStatus403(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 403"),
      _responseStatus404(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 404"),
      _responseStatus410(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 410"),
      _responseStatus4xx(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 4XX"),
      _responseStatus500(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 500"),
      _responseStatus502(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 502"),
      _responseStatus503(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 503"),
      _responseStatus504(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 504"),
      _responseStatus5xx(isProxy ? "PROXY "
                                 : "ORIGIN "
                                   "SERVER TRANS RESPONSE STATUS 5XX"),
      _responseStatusOther(isProxy ? "PROXY "
                                   : "ORIGIN "
                                     "SERVER TRANS RESPONSE STATUS OTHER"),
      _isProxy(isProxy) {}

HttpServerSimpleCounters::~HttpServerSimpleCounters() {}

void HttpServerSimpleCounters::log(ESB::Logger &logger, ESB::Logger::Severity severity) const {
  _successfulTransactions.log(logger, severity);
  _failedTransactions.log(logger, severity);
  _requestHeaderBeginError.log(logger, severity);
  _requestHeaderReceiveError.log(logger, severity);
  _requestBodyReceiveError.log(logger, severity);
  _requestBodySizeError.log(logger, severity);
  _responseHeaderSendError.log(logger, severity);
  _responseBodySendError.log(logger, severity);
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

ESB::PerformanceCounter &HttpServerSimpleCounters::successfulTransactions() { return _successfulTransactions; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::successfulTransactions() const {
  return _successfulTransactions;
}

ESB::PerformanceCounter &HttpServerSimpleCounters::failedTransactions() { return _failedTransactions; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::failedTransactions() const { return _failedTransactions; }

ESB::PerformanceCounter &HttpServerSimpleCounters::requestHeaderBeginError() { return _requestHeaderBeginError; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::requestHeaderBeginError() const {
  return _requestHeaderBeginError;
}

ESB::PerformanceCounter &HttpServerSimpleCounters::requestHeaderReceiveError() { return _requestHeaderReceiveError; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::requestHeaderReceiveError() const {
  return _requestHeaderReceiveError;
}

ESB::PerformanceCounter &HttpServerSimpleCounters::requestBodyReceiveError() { return _requestBodyReceiveError; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::requestBodyReceiveError() const {
  return _requestBodyReceiveError;
}

ESB::PerformanceCounter &HttpServerSimpleCounters::requestBodySizeError() { return _requestBodySizeError; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::requestBodySizeError() const { return _requestBodySizeError; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseHeaderSendError() { return _responseHeaderSendError; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseHeaderSendError() const {
  return _responseHeaderSendError;
}

ESB::PerformanceCounter &HttpServerSimpleCounters::responseBodySendError() { return _responseBodySendError; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseBodySendError() const {
  return _responseBodySendError;
}

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus1xx() { return _responseStatus1xx; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus1xx() const { return _responseStatus1xx; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus200() { return _responseStatus200; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus200() const { return _responseStatus200; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus201() { return _responseStatus201; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus201() const { return _responseStatus201; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus202() { return _responseStatus202; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus202() const { return _responseStatus202; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus204() { return _responseStatus204; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus204() const { return _responseStatus204; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus2xx() { return _responseStatus2xx; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus2xx() const { return _responseStatus2xx; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus304() { return _responseStatus304; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus304() const { return _responseStatus304; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus3xx() { return _responseStatus3xx; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus3xx() const { return _responseStatus3xx; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus400() { return _responseStatus400; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus400() const { return _responseStatus400; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus401() { return _responseStatus401; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus401() const { return _responseStatus401; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus403() { return _responseStatus403; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus403() const { return _responseStatus403; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus404() { return _responseStatus404; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus404() const { return _responseStatus404; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus410() { return _responseStatus410; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus410() const { return _responseStatus410; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus4xx() { return _responseStatus4xx; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus4xx() const { return _responseStatus4xx; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus500() { return _responseStatus500; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus500() const { return _responseStatus500; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus502() { return _responseStatus502; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus502() const { return _responseStatus502; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus503() { return _responseStatus503; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus503() const { return _responseStatus503; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus504() { return _responseStatus504; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus504() const { return _responseStatus504; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus5xx() { return _responseStatus5xx; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatus5xx() const { return _responseStatus5xx; }

ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatusOther() { return _responseStatusOther; }

const ESB::PerformanceCounter &HttpServerSimpleCounters::responseStatusOther() const { return _responseStatusOther; }

}  // namespace ES
