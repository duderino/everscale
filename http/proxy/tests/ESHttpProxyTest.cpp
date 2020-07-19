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

#include <gtest/gtest.h>

using namespace ES;

class HttpProxyTest : public ::testing::Test {
 public:
  HttpProxyTest() : _originListener("origin-listener"), _proxyListener("proxy-listener") {}

  // Sets up the test fixture.
  virtual void SetUp() { HttpLoadgenContext::Reset(); }

 protected:
  EphemeralListener _originListener;
  EphemeralListener _proxyListener;
};

TEST_F(HttpProxyTest, DISABLED_LargeChunks) {
  HttpTestParams params;
  params.connections(1)
      .iterations(1)
      .clientThreads(1)
      .proxyThreads(1)
      .originThreads(1)
      .requestSize(HttpConfig::Instance().ioBufferSize() * 42)
      .responseSize(HttpConfig::Instance().ioBufferSize() * 42)
      .logLevel(ESB::Logger::Debug);

  HttpFixedRouter router(_originListener.localDestination());
  HttpLoadgenHandler loadgenHandler(params);
  HttpRoutingProxyHandler proxyHandler(router);
  HttpOriginHandler originHandler(params);
  HttpIntegrationTest test(params, _originListener, _proxyListener, loadgenHandler, proxyHandler, originHandler);

  EXPECT_EQ(ESB_SUCCESS, test.run());
  EXPECT_EQ(params.connections() * params.iterations(), test.clientCounters().getSuccesses()->queries());
  EXPECT_EQ(0, test.clientCounters().getFailures()->queries());
}

TEST_F(HttpProxyTest, ClientToServer) {
  HttpTestParams params;
  params.connections(50)
      .iterations(50)
      .clientThreads(2)
      .proxyThreads(0)
      .originThreads(2)
      .requestSize(1024)
      .responseSize(1024)
      .logLevel(ESB::Logger::Warning);

  HttpLoadgenHandler loadgenHandler(params);
  HttpOriginHandler originHandler(params);
  HttpIntegrationTest test(params, _originListener, loadgenHandler, originHandler);

  EXPECT_EQ(ESB_SUCCESS, test.run());
  EXPECT_EQ(params.connections() * params.iterations(), test.clientCounters().getSuccesses()->queries());
  EXPECT_EQ(0, test.clientCounters().getFailures()->queries());
}

TEST_F(HttpProxyTest, ClientToProxyToServer) {
  HttpTestParams params;
  params.connections(50)
      .iterations(50)
      .clientThreads(2)
      .proxyThreads(2)
      .originThreads(2)
      .requestSize(1024)
      .responseSize(1024)
      .logLevel(ESB::Logger::Warning);

  HttpFixedRouter router(_originListener.localDestination());
  HttpLoadgenHandler loadgenHandler(params);
  HttpRoutingProxyHandler proxyHandler(router);
  HttpOriginHandler originHandler(params);
  HttpIntegrationTest test(params, _originListener, _proxyListener, loadgenHandler, proxyHandler, originHandler);

  EXPECT_EQ(ESB_SUCCESS, test.run());
  EXPECT_EQ(params.connections() * params.iterations(), test.clientCounters().getSuccesses()->queries());
  EXPECT_EQ(0, test.clientCounters().getFailures()->queries());
}

TEST_F(HttpProxyTest, EmptyChunkedRequestAndResponse) {
  HttpTestParams params;
  params.connections(50)
      .iterations(50)
      .clientThreads(2)
      .proxyThreads(2)
      .originThreads(2)
      .requestSize(0)
      .responseSize(0)
      .logLevel(ESB::Logger::Warning);

  HttpFixedRouter router(_originListener.localDestination());
  HttpLoadgenHandler loadgenHandler(params);
  HttpRoutingProxyHandler proxyHandler(router);
  HttpOriginHandler originHandler(params);
  HttpIntegrationTest test(params, _originListener, _proxyListener, loadgenHandler, proxyHandler, originHandler);

  EXPECT_EQ(ESB_SUCCESS, test.run());
  EXPECT_EQ(params.connections() * params.iterations(), test.clientCounters().getSuccesses()->queries());
  EXPECT_EQ(0, test.clientCounters().getFailures()->queries());
}

TEST_F(HttpProxyTest, EmptyChunkedRequest) {
  HttpTestParams params;
  params.connections(50)
      .iterations(50)
      .clientThreads(2)
      .proxyThreads(2)
      .originThreads(2)
      .requestSize(0)
      .responseSize(1024)
      .logLevel(ESB::Logger::Warning);

  HttpFixedRouter router(_originListener.localDestination());
  HttpLoadgenHandler loadgenHandler(params);
  HttpRoutingProxyHandler proxyHandler(router);
  HttpOriginHandler originHandler(params);
  HttpIntegrationTest test(params, _originListener, _proxyListener, loadgenHandler, proxyHandler, originHandler);

  EXPECT_EQ(ESB_SUCCESS, test.run());
  EXPECT_EQ(params.connections() * params.iterations(), test.clientCounters().getSuccesses()->queries());
  EXPECT_EQ(0, test.clientCounters().getFailures()->queries());
}

TEST_F(HttpProxyTest, EmptyChunkedResponse) {
  HttpTestParams params;
  params.connections(50)
      .iterations(50)
      .clientThreads(2)
      .proxyThreads(2)
      .originThreads(2)
      .requestSize(1024)
      .responseSize(0)
      .logLevel(ESB::Logger::Warning);

  HttpFixedRouter router(_originListener.localDestination());
  HttpLoadgenHandler loadgenHandler(params);
  HttpRoutingProxyHandler proxyHandler(router);
  HttpOriginHandler originHandler(params);
  HttpIntegrationTest test(params, _originListener, _proxyListener, loadgenHandler, proxyHandler, originHandler);

  EXPECT_EQ(ESB_SUCCESS, test.run());
  EXPECT_EQ(params.connections() * params.iterations(), test.clientCounters().getSuccesses()->queries());
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

TEST_F(HttpProxyTest, SmallChunks) {
  HttpTestParams params;
  params.connections(50)
      .iterations(50)
      .clientThreads(2)
      .proxyThreads(2)
      .originThreads(2)
      .requestSize(1024)
      .responseSize(1024)
      .logLevel(ESB::Logger::Warning);
  const ESB::UInt32 maxChunkSize = 42;

  HttpFixedRouter router(_originListener.localDestination());
  HttpSmallChunkLoadgenHandler loadgenHandler(params, maxChunkSize);
  HttpRoutingProxyHandler proxyHandler(router);
  HttpSmallChunkOriginHandler originHandler(params, maxChunkSize);
  HttpIntegrationTest test(params, _originListener, _proxyListener, loadgenHandler, proxyHandler, originHandler);

  EXPECT_EQ(ESB_SUCCESS, test.run());
  EXPECT_EQ(params.connections() * params.iterations(), test.clientCounters().getSuccesses()->queries());
  EXPECT_EQ(0, test.clientCounters().getFailures()->queries());
}
