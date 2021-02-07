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

static ESB::SimpleFileLogger TestLogger(stdout, ESB::Logger::Info);

class HttpProxyNegativeTest
    : public ::testing::TestWithParam<std::tuple<ESB::UInt32, bool, bool, HttpTestParams::DisruptTransaction>> {
 public:
  HttpProxyNegativeTest() {}

  virtual ~HttpProxyNegativeTest() {}

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

  ESB_DISABLE_AUTO_COPY(HttpProxyNegativeTest);
};

// body-size variations X use content-length header if true X use secure if true X failure type
INSTANTIATE_TEST_SUITE_P(Variants, HttpProxyNegativeTest,
                         ::testing::Combine(::testing::Values(0, 1024, HttpConfig::Instance().ioBufferSize() * 42),
                                            ::testing::Values(false, true), ::testing::Values(false, true),
                                            ::testing::Values(HttpTestParams::STALL_SERVER_RECV_HEADERS,
                                                              HttpTestParams::STALL_SERVER_RECV_BODY,
                                                              HttpTestParams::STALL_SERVER_SEND_HEADERS,
                                                              HttpTestParams::STALL_SERVER_SEND_BODY)));

TEST_P(HttpProxyNegativeTest, OriginIdleTimeout) {
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
      .logLevel(ESB::Logger::Warning)
      .disruptTransaction(std::get<3>(GetParam()))
      .proxyTimeoutMsec(10)
      .originTimeoutMsec(1000)
      .clientTimeoutMsec(1000);

  EphemeralListener originListener("origin-listener", params.secure());
  EphemeralListener proxyListener("proxy-listener", params.secure());
  HttpFixedRouter router(originListener.localDestination());
  HttpLoadgenHandler loadgenHandler(params);
  HttpRoutingProxyHandler proxyHandler(router);
  HttpOriginHandler originHandler(params);
  HttpIntegrationTest test(params, originListener, proxyListener, loadgenHandler, proxyHandler, originHandler);

  EXPECT_EQ(ESB_SUCCESS, test.run());
  EXPECT_EQ(0, test.clientCounters().getSuccesses()->queries());
  EXPECT_EQ(params.connections() * params.requestsPerConnection(), test.clientCounters().getFailures()->queries());
}
