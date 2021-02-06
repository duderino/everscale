#ifndef ES_HTTP_INTEGRATION_TEST_H
#include "ESHttpIntegrationTest.h"
#endif

#ifndef ES_HTTP_TEST_PARAMS_H
#include <ESHttpTestParams.h>
#endif

#ifndef ES_EPHEMERAL_LISTENER_H
#include <ESEphemeralListener.h>
#endif

#ifndef ES_HTTP_LOADGEN_HANDLER_H
#include <ESHttpLoadgenHandler.h>
#endif

#ifndef ES_HTTP_ROUTING_PROXY_HANDLER_H
#include <ESHttpRoutingProxyHandler.h>
#endif

#ifndef ES_HTTP_ORIGIN_HANDLER_H
#include <ESHttpOriginHandler.h>
#endif

#ifndef ES_HTTP_FIXED_ROUTER_H
#include "ESHttpFixedRouter.h"
#endif

#ifndef ES_HTTP_LOADGEN_CONTEXT_H
#include <ESHttpLoadgenContext.h>
#endif

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

#ifndef ES_HTTP_CONFIG_H
#include <ESHttpConfig.h>
#endif

#ifndef ESB_CLIENT_TLS_SOCKET_H
#include <ESBClientTLSSocket.h>
#endif

#ifndef ESB_SERVER_TLS_SOCKET_H
#include <ESBServerTLSSocket.h>
#endif

#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
#endif

#ifndef ESB_SYSTEM_TIME_SOURCE_H
#include <ESBSystemTimeSource.h>
#endif

#include <gtest/gtest.h>

using namespace ES;

static ESB::SimpleFileLogger TestLogger(stdout, ESB::Logger::Warning);

class HttpProxyTest : public ::testing::TestWithParam<std::tuple<bool>> {
 public:
  HttpProxyTest(){};

  virtual ~HttpProxyTest(){};

  // Run before each HttpProxyTest test case
  virtual void SetUp() { HttpLoadgenContext::Reset(); }

  // Run after each HttpProxyTest test case
  virtual void TearDown() {}

  // Run before all HttpProxyTest test cases
  static void SetUpTestSuite() {
    ESB::Logger::SetInstance(&TestLogger);

    HttpTestParams params;
    ESB::Error error = ESB::ClientTLSSocket::Initialize(params.caPath(), params.maxVerifyDepth());
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot initialize client TLS support");
      LogCurrentWorkingDirectory(ESB::Logger::Err);
      exit(error);
    }

    error = ESB::ServerTLSSocket::Initialize(params.serverKeyPath(), params.serverCertPath());
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot initialize server TLS support");
      LogCurrentWorkingDirectory(ESB::Logger::Err);
      exit(error);
    }
  }

  // Run after all HttpProxyTest test cases
  static void TearDownTestSuite() {
    ESB::ClientTLSSocket::Destroy();
    ESB::ServerTLSSocket::Destroy();
    ESB::Logger::SetInstance(NULL);
  }

  ESB_DISABLE_AUTO_COPY(HttpProxyTest);
};

// use secure if true
INSTANTIATE_TEST_SUITE_P(Variants, HttpProxyTest, ::testing::Combine(::testing::Values(false, true)));

TEST_P(HttpProxyTest, ClientToServer) {
  HttpTestParams params;
  params.connections(50)
      .requestsPerConnection(50)
      .clientThreads(2)
      .proxyThreads(0)
      .originThreads(2)
      .requestSize(1024)
      .responseSize(1024)
      .hostHeader("test.server.everscale.com")
      .secure(std::get<0>(GetParam()))
      .logLevel(ESB::Logger::Warning);

  EphemeralListener originListener("origin-listener", params.secure());
  HttpLoadgenHandler loadgenHandler(params);
  HttpOriginHandler originHandler(params);
  HttpIntegrationTest test(params, originListener, loadgenHandler, originHandler);

  EXPECT_EQ(ESB_SUCCESS, test.run());
  EXPECT_EQ(params.connections() * params.requestsPerConnection(), test.clientCounters().getSuccesses()->queries());
  EXPECT_EQ(0, test.clientCounters().getFailures()->queries());
}

TEST_P(HttpProxyTest, ClientToProxyToServer) {
  HttpTestParams params;
  params.connections(50)
      .requestsPerConnection(50)
      .clientThreads(2)
      .proxyThreads(2)
      .originThreads(2)
      .requestSize(1024)
      .responseSize(1024)
      .hostHeader("test.server.everscale.com")
      .secure(std::get<0>(GetParam()))
      .logLevel(ESB::Logger::Warning);

  EphemeralListener originListener("origin-listener", params.secure());
  EphemeralListener proxyListener("proxy-listener", params.secure());
  HttpFixedRouter router(originListener.localDestination());
  HttpLoadgenHandler loadgenHandler(params);
  HttpRoutingProxyHandler proxyHandler(router);
  HttpOriginHandler originHandler(params);
  HttpIntegrationTest test(params, originListener, proxyListener, loadgenHandler, proxyHandler, originHandler);

  EXPECT_EQ(ESB_SUCCESS, test.run());
  EXPECT_EQ(params.connections() * params.requestsPerConnection(), test.clientCounters().getSuccesses()->queries());
  EXPECT_EQ(0, test.clientCounters().getFailures()->queries());
}

class HttpSmallChunkOriginHandler : public HttpOriginHandler {
 public:
  HttpSmallChunkOriginHandler(const HttpTestParams &params, ESB::UInt32 maxChunkSize)
      : HttpOriginHandler(params), _counter(), _maxChunkSize(maxChunkSize) {}

  virtual ~HttpSmallChunkOriginHandler() {}

  virtual ESB::Error offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                       ESB::UInt32 *bytesAvailable) {
    ESB::Error error = HttpOriginHandler::offerResponseBody(multiplexer, stream, bytesAvailable);
    if (ESB_SUCCESS != error) {
      return error;
    }

    ESB::UInt32 iteration = (ESB::UInt32)_counter.inc();
    *bytesAvailable = MIN(*bytesAvailable, iteration % _maxChunkSize + 1);
    return ESB_SUCCESS;
  }

 private:
  // Disabled
  HttpSmallChunkOriginHandler(const HttpSmallChunkOriginHandler &);
  void operator=(const HttpSmallChunkOriginHandler &);

  ESB::SharedInt _counter;
  const ESB::UInt32 _maxChunkSize;
};

class HttpSmallChunkLoadgenHandler : public HttpLoadgenHandler {
 public:
  HttpSmallChunkLoadgenHandler(const HttpTestParams &params, ESB::UInt32 maxChunkSize)
      : HttpLoadgenHandler(params), _counter(), _maxChunkSize(maxChunkSize) {}

  virtual ~HttpSmallChunkLoadgenHandler() {}

  virtual ESB::Error offerResponseBody(HttpMultiplexer &multiplexer, HttpClientStream &stream,
                                       ESB::UInt32 *bytesAvailable) {
    ESB::Error error = HttpLoadgenHandler::offerRequestBody(multiplexer, stream, bytesAvailable);
    if (ESB_SUCCESS != error) {
      return error;
    }

    ESB::UInt32 iteration = (ESB::UInt32)_counter.inc();
    *bytesAvailable = MIN(*bytesAvailable, iteration % _maxChunkSize + 1);
    return ESB_SUCCESS;
  }

 private:
  // Disabled
  HttpSmallChunkLoadgenHandler(const HttpSmallChunkLoadgenHandler &);
  void operator=(const HttpSmallChunkLoadgenHandler &);

  ESB::SharedInt _counter;
  const ESB::UInt32 _maxChunkSize;
};

TEST_P(HttpProxyTest, SmallChunks) {
  HttpTestParams params;
  params.connections(50)
      .requestsPerConnection(50)
      .clientThreads(2)
      .proxyThreads(2)
      .originThreads(2)
      .requestSize(1024)
      .responseSize(1024)
      .hostHeader("test.server.everscale.com")
      .secure(std::get<0>(GetParam()))
      .logLevel(ESB::Logger::Warning);
  const ESB::UInt32 maxChunkSize = 42;

  EphemeralListener originListener("origin-listener", params.secure());
  EphemeralListener proxyListener("proxy-listener", params.secure());
  HttpFixedRouter router(originListener.localDestination());
  HttpSmallChunkLoadgenHandler loadgenHandler(params, maxChunkSize);
  HttpRoutingProxyHandler proxyHandler(router);
  HttpSmallChunkOriginHandler originHandler(params, maxChunkSize);
  HttpIntegrationTest test(params, originListener, proxyListener, loadgenHandler, proxyHandler, originHandler);

  EXPECT_EQ(ESB_SUCCESS, test.run());
  EXPECT_EQ(params.connections() * params.requestsPerConnection(), test.clientCounters().getSuccesses()->queries());
  EXPECT_EQ(0, test.clientCounters().getFailures()->queries());
}

class HttpProxyTestMessageBody : public ::testing::TestWithParam<std::tuple<ESB::UInt32, bool, bool>> {
 public:
  HttpProxyTestMessageBody() {}

  virtual ~HttpProxyTestMessageBody() {}

  // Run before each HttpProxyTestMessageBody test case
  virtual void SetUp() { HttpLoadgenContext::Reset(); }

  // Run after each HttpProxyTestMessageBody test case
  virtual void TearDown() {}

  // Run before all HttpProxyTestMessageBody test cases
  static void SetUpTestSuite() {
    ESB::Logger::SetInstance(&TestLogger);

    HttpTestParams params;
    ESB::Error error = ESB::ClientTLSSocket::Initialize(params.caPath(), params.maxVerifyDepth());
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot initialize client TLS support");
      LogCurrentWorkingDirectory(ESB::Logger::Err);
      exit(error);
    }

    error = ESB::ServerTLSSocket::Initialize(params.serverKeyPath(), params.serverCertPath());
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot initialize server TLS support");
      LogCurrentWorkingDirectory(ESB::Logger::Err);
      exit(error);
    }
  }

  // Run after all HttpProxyTestMessageBody test cases
  static void TearDownTestSuite() {
    ESB::ClientTLSSocket::Destroy();
    ESB::ServerTLSSocket::Destroy();
    ESB::Logger::SetInstance(NULL);
  }

  ESB_DISABLE_AUTO_COPY(HttpProxyTestMessageBody);
};

// body-size variations X use content-length header if true X use secure if
INSTANTIATE_TEST_SUITE_P(Variants, HttpProxyTestMessageBody,
                         ::testing::Combine(::testing::Values(0, 1024, HttpConfig::Instance().ioBufferSize() * 42),
                                            ::testing::Values(false, true), ::testing::Values(false, true)));

TEST_P(HttpProxyTestMessageBody, BodySizes) {
  HttpTestParams params;
  params.connections(50)
      .requestsPerConnection(50)
      .clientThreads(2)
      .proxyThreads(2)
      .originThreads(2)
      .requestSize(std::get<0>(GetParam()))
      .responseSize(std::get<0>(GetParam()))
      .useContentLengthHeader(std::get<1>(GetParam()))
      .hostHeader("test.server.everscale.com")
      .secure(std::get<2>(GetParam()))
      .originTimeoutMsec(10 * 1000)
      .proxyTimeoutMsec(10 * 1000)
      .clientTimeoutMsec(10 * 1000)
      .logLevel(ESB::Logger::Warning);

  EphemeralListener originListener("origin-listener", params.secure());
  EphemeralListener proxyListener("proxy-listener", params.secure());
  HttpFixedRouter router(originListener.localDestination());
  HttpLoadgenHandler loadgenHandler(params);
  HttpRoutingProxyHandler proxyHandler(router);
  HttpOriginHandler originHandler(params);
  HttpIntegrationTest test(params, originListener, proxyListener, loadgenHandler, proxyHandler, originHandler);

  EXPECT_EQ(ESB_SUCCESS, test.run());
  EXPECT_EQ(params.connections() * params.requestsPerConnection(), test.clientCounters().getSuccesses()->queries());
  EXPECT_EQ(0, test.clientCounters().getFailures()->queries());
}
