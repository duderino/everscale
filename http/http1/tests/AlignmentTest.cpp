#ifndef ES_HTTP_SERVER_SOCKET_H
#include <ESHttpServerSocket.h>
#endif

#ifndef ES_HTTP_SERVER_TRANSACTION_H
#include <ESHttpServerTransaction.h>
#endif

#ifndef ES_HTTP_CLIENT_SOCKET_FACTORY_H
#include <ESHttpClientSocketFactory.h>
#endif

#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#include <gtest/gtest.h>

using namespace ES;

TEST(HttpCommon, Alignment) {
  ESB::UInt32 cacheLineSize = ESB::SystemConfig::Instance().cacheLineSize();
  EXPECT_EQ(cacheLineSize, sizeof(HttpRequestUriParser));
  EXPECT_EQ(cacheLineSize * 2, sizeof(HttpRequestParser));
  EXPECT_EQ(cacheLineSize, sizeof(HttpResponseParser));
  EXPECT_EQ(cacheLineSize, sizeof(HttpResponseFormatter));
  EXPECT_EQ(cacheLineSize, sizeof(HttpRequestFormatter));

  EXPECT_EQ(48, sizeof(ESB::DiscardAllocator));
  EXPECT_EQ(24, sizeof(ESB::EmbeddedListElement));
  EXPECT_EQ(16, sizeof(ESB::Date));
  EXPECT_EQ(24, sizeof(ESB::SocketAddress));
  EXPECT_EQ(cacheLineSize, sizeof(ESB::Buffer));

  EXPECT_EQ(320, sizeof(HttpTransaction));

  EXPECT_EQ(2048, sizeof(HttpServerTransaction));
  EXPECT_EQ(2048, sizeof(HttpClientTransaction));

  EXPECT_EQ(24, sizeof(ESB::MultiplexedSocket));
  EXPECT_EQ(64, sizeof(ESB::ConnectedTCPSocket));
  EXPECT_EQ(4, sizeof(int));
  EXPECT_EQ(cacheLineSize * 2, sizeof(HttpClientSocket));

  // TODO align after transactions are broken out
  // EXPECT_EQ(2048, sizeof(HttpServerSocket));
}
