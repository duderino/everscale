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

class HttpProxyNegativeTest
    : public ::testing::TestWithParam<
          std::tuple<ESB::UInt32, bool, bool, HttpTestParams::DisruptTransaction, ESB::UInt32, ESB::UInt32>> {
 public:
  HttpProxyNegativeTest() {}

  virtual ~HttpProxyNegativeTest() {}

  // Run before each HttpProxyTestMessageBody test case
  virtual void SetUp() { HttpLoadgenContext::Reset(); }

  // Run after each HttpProxyTestMessageBody test case
  virtual void TearDown() {}

  // Run before all HttpProxyTestMessageBody test cases
  static void SetUpTestSuite() { ESB::Logger::SetInstance(&TestLogger); }

  // Run after all HttpProxyTestMessageBody test cases
  static void TearDownTestSuite() { ESB::Logger::SetInstance(NULL); }

  ESB_DISABLE_AUTO_COPY(HttpProxyNegativeTest);
};

TEST_P(HttpProxyNegativeTest, FailCleanly) {
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
      .logLevel(ESB::Logger::Notice)
      .disruptTransaction(std::get<3>(GetParam()))
      .proxyTimeoutMsec(std::get<5>(GetParam()))
      .originTimeoutMsec(1000)
      .clientTimeoutMsec(std::get<4>(GetParam()));

  EphemeralListener originListener("origin-listener", params.secure());
  EphemeralListener proxyListener("proxy-listener", params.secure());
  HttpFixedRouter router(originListener.localDestination());
  HttpLoadgenHandler loadgenHandler(params);
  HttpRoutingProxyHandler proxyHandler(router);
  HttpOriginHandler originHandler(params);
  HttpIntegrationTest test(params, originListener, proxyListener, loadgenHandler, proxyHandler, originHandler);

  ASSERT_EQ(ESB_SUCCESS, test.loadDefaultTLSContexts());
  ASSERT_EQ(ESB_SUCCESS, test.run());
  ASSERT_EQ(0, test.client().clientCounters().getSuccesses()->queries());
  ASSERT_EQ(params.connections() * params.requestsPerConnection(),
            test.client().clientCounters().getFailures()->queries());
}

// TODO - Start 223: IdleOrigin/HttpProxyNegativeTest.FailCleanly/(1373526,false,true,3,1000,10) - timeout? deadlock?
// after STALL_SERVER_SEND_HEADERS, chunked transfer encoding, TLS

// body-size variations X use content-length header if true X use secure if true X failure type X client timeout msec X
// proxy timeout msec
INSTANTIATE_TEST_SUITE_P(BadOrigin, HttpProxyNegativeTest,
                         ::testing::Combine(::testing::Values(0, 1024, HttpConfig::Instance().ioBufferSize() * 42),
                                            ::testing::Values(false, true), ::testing::Values(false, true),
                                            ::testing::Values(HttpTestParams::CLOSE_SERVER_RECV_HEADERS,
                                                              HttpTestParams::CLOSE_SERVER_RECV_BODY,
                                                              HttpTestParams::CLOSE_SERVER_SEND_HEADERS,
                                                              HttpTestParams::CLOSE_SERVER_SEND_BODY),
                                            ::testing::Values(10 * 1000), ::testing::Values(10 * 1000)));

// body-size variations X use content-length header if true X use secure if true X failure type X client timeout msec X
// proxy timeout msec
INSTANTIATE_TEST_SUITE_P(BadClient, HttpProxyNegativeTest,
                         ::testing::Combine(::testing::Values(1024, HttpConfig::Instance().ioBufferSize() * 42),
                                            ::testing::Values(false, true), ::testing::Values(false, true),
                                            ::testing::Values(HttpTestParams::CLOSE_CLIENT_SEND_BODY,
                                                              HttpTestParams::CLOSE_CLIENT_RECV_HEADERS,
                                                              HttpTestParams::CLOSE_CLIENT_RECV_BODY),
                                            ::testing::Values(10 * 1000), ::testing::Values(10 * 1000)));

// body-size variations X use content-length header if true X use secure if true X failure type X client timeout msec X
// proxy timeout msec
INSTANTIATE_TEST_SUITE_P(BadClient2, HttpProxyNegativeTest,
                         ::testing::Combine(::testing::Values(0), ::testing::Values(false, true),
                                            ::testing::Values(false, true),
                                            ::testing::Values(HttpTestParams::CLOSE_CLIENT_SEND_BODY,
                                                              HttpTestParams::CLOSE_CLIENT_RECV_HEADERS),
                                            ::testing::Values(10 * 1000), ::testing::Values(10 * 1000)));

// body-size variations X use content-length header if true X use secure if true X failure type X client timeout msec X
// proxy timeout msec
INSTANTIATE_TEST_SUITE_P(IdleOrigin, HttpProxyNegativeTest,
                         ::testing::Combine(::testing::Values(0, 1024, HttpConfig::Instance().ioBufferSize() * 42),
                                            ::testing::Values(false, true), ::testing::Values(false, true),
                                            ::testing::Values(HttpTestParams::STALL_SERVER_RECV_HEADERS,
                                                              HttpTestParams::STALL_SERVER_RECV_BODY,
                                                              HttpTestParams::STALL_SERVER_SEND_HEADERS,
                                                              HttpTestParams::STALL_SERVER_SEND_BODY),
                                            ::testing::Values(10 * 1000), ::testing::Values(10)));

// body-size variations X use content-length header if true X use secure if true X failure type X client timeout msec X
// proxy timeout msec
INSTANTIATE_TEST_SUITE_P(IdleClient, HttpProxyNegativeTest,
                         ::testing::Combine(::testing::Values(1024, HttpConfig::Instance().ioBufferSize() * 42),
                                            ::testing::Values(false, true), ::testing::Values(false, true),
                                            ::testing::Values(HttpTestParams::STALL_CLIENT_SEND_BODY,
                                                              HttpTestParams::STALL_CLIENT_RECV_HEADERS,
                                                              HttpTestParams::STALL_CLIENT_RECV_BODY),
                                            ::testing::Values(20), ::testing::Values(10)));

// body-size variations X use content-length header if true X use secure if true X failure type X client timeout msec X
// proxy timeout msec
INSTANTIATE_TEST_SUITE_P(IdleClient2, HttpProxyNegativeTest,
                         ::testing::Combine(::testing::Values(0), ::testing::Values(false, true),
                                            ::testing::Values(false, true),
                                            ::testing::Values(HttpTestParams::STALL_CLIENT_SEND_BODY,
                                                              HttpTestParams::STALL_CLIENT_RECV_HEADERS),
                                            ::testing::Values(20), ::testing::Values(10)));
