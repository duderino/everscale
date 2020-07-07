#ifndef ES_HTTP_INTEGRATION_TEST_H
#define ES_HTTP_INTEGRATION_TEST_H

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
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

#ifndef ES_HTTP_ORIGIN_HANDLER_H
#include <ESHttpOriginHandler.h>
#endif

#ifndef ES_HTTP_LOADGEN_HANDLER_H
#include <ESHttpLoadgenHandler.h>
#endif

#ifndef ES_HTTP_ROUTING_PROXY_HANDLER_H
#include <ESHttpRoutingProxyHandler.h>
#endif

namespace ES {

class HttpIntegrationTest {
 public:
  class TestParams {
   public:
    TestParams();

    virtual ~TestParams();

    /**
     * Override test params with command line arguments
     *
     * @param argc the argc passed to main()
     * @param argv the argv passed to main()
     * @return ESB_SUCCESS if successful, another error code otherwise.
     */
    ESB::Error override(int argc, char **argv);

    inline TestParams &clientThreads(ESB::UInt32 clientThreads) {
      _clientThreads = clientThreads;
      return *this;
    }

    inline TestParams &originThreads(ESB::UInt32 originThreads) {
      _originThreads = originThreads;
      return *this;
    }

    inline TestParams &proxyThreads(ESB::UInt32 proxyThreads) {
      _proxyThreads = proxyThreads;
      return *this;
    }

    inline TestParams &connections(ESB::UInt32 connections) {
      _connections = connections;
      return *this;
    }

    inline TestParams &iterations(ESB::UInt32 iterations) {
      _iterations = iterations;
      return *this;
    }

    TestParams &requestSize(ESB::UInt32 requestSize);

    TestParams &responseSize(ESB::UInt32 responseSize);

    inline TestParams &reuseConnections(bool reuseConnections) {
      _reuseConnections = reuseConnections;
      return *this;
    }

    inline TestParams &logLevel(ESB::Logger::Severity logLevel) {
      _logLevel = logLevel;
      return *this;
    }

    inline TestParams &destinationAddress(const char *destinationAddress) {
      _destinationAddress = destinationAddress;
      return *this;
    }

    inline TestParams &hostHeader(const char *hostHeader) {
      _hostHeader = hostHeader;
      return *this;
    }

    inline TestParams &method(const char *method) {
      _method = method;
      return *this;
    }

    inline TestParams &contentType(const char *contentType) {
      _contentType = contentType;
      return *this;
    }

    inline TestParams &absPath(const char *absPath) {
      _absPath = absPath;
      return *this;
    }

    inline ESB::UInt32 clientThreads() const { return _clientThreads; }

    inline ESB::UInt32 originThreads() const { return _originThreads; }

    inline ESB::UInt32 proxyThreads() const { return _proxyThreads; }

    inline ESB::UInt32 connections() const { return _connections; }

    inline ESB::UInt32 iterations() const { return _iterations; }

    inline ESB::UInt32 requestSize() const { return _requestSize; }

    inline ESB::UInt32 responseSize() const { return _responseSize; }

    inline bool reuseConnections() const { return _reuseConnections; }

    inline ESB::Logger::Severity logLevel() const { return _logLevel; }

    inline const char *destinationAddress() const { return _destinationAddress; }

    inline const char *hostHeader() const { return _hostHeader; }

    inline const char *method() const { return _method; }

    inline const char *contentType() const { return _contentType; }

    inline const char *absPath() const { return _absPath; }

    const unsigned char *requestBody();

    const unsigned char *responseBody();

   private:
    ESB::UInt32 _clientThreads;
    ESB::UInt32 _originThreads;
    ESB::UInt32 _proxyThreads;
    ESB::UInt32 _connections;
    ESB::UInt32 _iterations;  // requests per connection
    ESB::UInt32 _requestSize;
    ESB::UInt32 _responseSize;
    bool _reuseConnections;
    ESB::Logger::Severity _logLevel;
    const char *_destinationAddress;
    const char *_hostHeader;
    const char *_method;
    const char *_contentType;
    const char *_absPath;
    const unsigned char *_requestBody;
    const unsigned char *_responseBody;
  };

  HttpIntegrationTest(TestParams &testParams);
  virtual ~HttpIntegrationTest();

  ESB::Error run();

  inline const HttpClientCounters &clientCounters() const { return _client.clientCounters(); }

 private:
  // Disabled
  HttpIntegrationTest(const HttpIntegrationTest &);
  void operator=(const HttpIntegrationTest &);

  /**
   * This router blindly forwards all requests to a given address.
   */
  class HttpFixedRouter : public HttpRouter {
   public:
    HttpFixedRouter(ESB::SocketAddress &destination);
    HttpFixedRouter();
    virtual ~HttpFixedRouter();

    virtual ESB::Error route(const HttpServerStream &serverStream, HttpClientTransaction &clientTransaction,
                             ESB::SocketAddress &destination);

    inline void setDestination(ESB::SocketAddress &destination) { _destination = destination; }

   private:
    // Disabled
    HttpFixedRouter(const HttpFixedRouter &);
    void operator=(const HttpFixedRouter &);

    ESB::SocketAddress _destination;
  };

  TestParams _params;
  ESB::SimpleFileLogger _logger;
  ESB::ListeningTCPSocket _originListener;
  ESB::ListeningTCPSocket _proxyListener;
  HttpOriginHandler _serverHandler;
  HttpServer _originServer;
  HttpLoadgenHandler _clientHandler;
  HttpClient _client;
  HttpFixedRouter _proxyRouter;
  HttpRoutingProxyHandler _proxyHandler;
  HttpProxy _proxyServer;
};

}  // namespace ES

#endif
