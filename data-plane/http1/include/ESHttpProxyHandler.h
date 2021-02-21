#ifndef ES_HTTP_PROXY_HANDLER_H
#define ES_HTTP_PROXY_HANDLER_H

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

#ifndef ES_HTTP_CLIENT_HANDLER_H
#include <ESHttpClientHandler.h>
#endif

namespace ES {

class HttpProxyHandler : public HttpServerHandler, public HttpClientHandler {
 public:
  HttpProxyHandler();
  virtual ~HttpProxyHandler();

 private:
  // Disabled
  HttpProxyHandler(const HttpProxyHandler &serverHandler);
  void operator=(const HttpProxyHandler &serverHandler);
};

}  // namespace ES

#endif
