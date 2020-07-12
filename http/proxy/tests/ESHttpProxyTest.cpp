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

TEST_F(HttpProxyTest, ClientToServer) {
  HttpTestParams params;
  params.connections(50)
      .iterations(50)
      .clientThreads(1)
      .proxyThreads(0)
      .originThreads(1)
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
      .clientThreads(1)
      .proxyThreads(1)
      .originThreads(1)
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
      .clientThreads(1)
      .proxyThreads(1)
      .originThreads(1)
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
      .clientThreads(1)
      .proxyThreads(1)
      .originThreads(1)
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
      .clientThreads(1)
      .proxyThreads(1)
      .originThreads(1)
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