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

#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
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
  HttpIntegrationTest(HttpTestParams &testParams, ESB::ListeningTCPSocket &originListener,
                      ESB::ListeningTCPSocket &_proxyListener, HttpClientHandler &clientHandler,
                      HttpProxyHandler &proxyHandler, HttpServerHandler &serverHandler);
  HttpIntegrationTest(HttpTestParams &testParams, ESB::ListeningTCPSocket &originListener,
                      HttpClientHandler &clientHandler, HttpServerHandler &serverHandler);
  virtual ~HttpIntegrationTest();

  ESB::Error run();

  inline const HttpClientCounters &clientCounters() const { return _client.clientCounters(); }

 private:
  // Disabled
  HttpIntegrationTest(const HttpIntegrationTest &);
  void operator=(const HttpIntegrationTest &);

  HttpTestParams &_params;
  ESB::ListeningTCPSocket &_proxyListener;
  ESB::ListeningTCPSocket &_originListener;
  HttpClientHandler &_clientHandler;
  HttpProxyHandler &_proxyHandler;
  HttpServerHandler &_originHandler;
  ESB::SimpleFileLogger _logger;
  HttpClient _client;
  HttpProxy _proxy;
  HttpServer _origin;
};

}  // namespace ES

#endif
