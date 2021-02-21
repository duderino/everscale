#ifndef ES_HTTP_SERVER_SOCKET_H
#include <ESHttpServerSocket.h>
#endif

#ifndef ES_HTTP_SERVER_TRANSACTION_H
#include <ESHttpServerTransaction.h>
#endif

#ifndef ES_HTTP_CLIENT_SOCKET_H
#include <ESHttpClientSocket.h>
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
  EXPECT_EQ(cacheLineSize, sizeof(HttpServerSocket));
  EXPECT_EQ(cacheLineSize, sizeof(HttpClientSocket));
  EXPECT_EQ(cacheLineSize, sizeof(HttpServerTransaction));
  EXPECT_EQ(cacheLineSize, sizeof(HttpClientTransaction));
}
