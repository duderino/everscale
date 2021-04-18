#ifndef ES_HTTP_INTEGRATION_TEST_H
#define ES_HTTP_INTEGRATION_TEST_H

#ifndef ES_HTTP_TEST_PARAMS_H
#include <ESHttpTestParams.h>
#endif

#ifndef ES_HTTP_SERVER_H
#include <ESHttpServer.h>
#endif

#ifndef ES_HTTP_CLIENT_H
#include <ESHttpClient.h>
#endif

#ifndef ES_HTTP_PROXY_H
#include <ESHttpProxy.h>
#endif

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

#ifndef ES_HTTP_CLIENT_HANDLER_H
#include <ESHttpClientHandler.h>
#endif

#ifndef ES_HTTP_PROXY_HANDLER_H
#include <ESHttpProxyHandler.h>
#endif

namespace ES {

class HttpIntegrationTest {
 public:
  HttpIntegrationTest(const HttpTestParams &testParams, ESB::ListeningSocket &originListener,
                      ESB::ListeningSocket &_proxyListener, HttpClientHandler &clientHandler,
                      HttpProxyHandler &proxyHandler, HttpServerHandler &serverHandler);
  HttpIntegrationTest(const HttpTestParams &testParams, ESB::ListeningSocket &originListener,
                      HttpClientHandler &clientHandler, HttpServerHandler &serverHandler);
  virtual ~HttpIntegrationTest();

  ESB::Error loadDefaultTLSContexts();
  ESB::Error run();

  inline HttpClient &client() { return _client; }
  inline const HttpClient &client() const { return _client; }

  inline HttpProxy &proxy() { return _proxy; }
  inline const HttpProxy &proxy() const { return _proxy; }

  inline HttpServer &origin() { return _origin; }
  inline const HttpServer &origin() const { return _origin; }

 private:
  const HttpTestParams &_params;
  ESB::ListeningSocket &_proxyListener;
  ESB::ListeningSocket &_originListener;
  HttpClientHandler &_clientHandler;
  HttpProxyHandler &_proxyHandler;
  HttpServerHandler &_originHandler;
  HttpClient _client;
  HttpProxy _proxy;
  HttpServer _origin;

  ESB_DISABLE_AUTO_COPY(HttpIntegrationTest);
};

extern void LogCurrentWorkingDirectory(ESB::Logger::Severity severity);

}  // namespace ES

#endif
