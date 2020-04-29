#ifndef ES_HTTP_ECHO_CLIENT_REQUEST_BUILDER_H
#include <ESHttpEchoClientRequestBuilder.h>
#endif

namespace ES {

ESB::Error HttpEchoClientRequestBuilder(const char *host, int port,
                                        const char *absPath, const char *method,
                                        const char *contentType,
                                        HttpClientTransaction *transaction) {
  if (0 == host || 0 == absPath || 0 == method || 0 == transaction) {
    return ESB_NULL_POINTER;
  }

  if (0 > port || 65536 <= port) {
    return ESB_INVALID_ARGUMENT;
  }

  HttpRequest &request = transaction->request();
  HttpRequestUri &requestUri = request.requestUri();

  requestUri.setType(HttpRequestUri::ES_URI_HTTP);
  requestUri.setAbsPath(absPath);
  request.setMethod(method);

  ESB::Error error =
      request.addHeader(transaction->allocator(), "Host", "%s:%d", host, port);

  if (ESB_SUCCESS != error) {
    return error;
  }

  if (contentType) {
    error = request.addHeader("Content-Type", contentType,
                              transaction->allocator());
    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  error = request.addHeader("Transfer-Encoding", "chunked",
                            transaction->allocator());

  if (ESB_SUCCESS != error) {
    return error;
  }

  // Body is hardcoded in HttpEchoClientHandler.cpp

  return ESB_SUCCESS;
}

}  // namespace ES
