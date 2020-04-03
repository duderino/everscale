#include <ESBBuffer.h>
#include <ESBConnectedTCPSocket.h>
#include <ESBDate.h>
#include <ESBDiscardAllocator.h>
#include <ESBEmbeddedListElement.h>
#include <ESBMultiplexedSocket.h>
#include <ESBSocketAddress.h>
#include <ESBSystemConfig.h>

#include <gtest/gtest.h>

using namespace ESB;

TEST(HttpCommon, Alignment) {
  UInt32 cacheLineSize = SystemConfig::Instance().cacheLineSize();
  EXPECT_EQ(48, sizeof(DiscardAllocator));
  EXPECT_EQ(24, sizeof(EmbeddedListElement));
  EXPECT_EQ(16, sizeof(Date));
  EXPECT_EQ(24, sizeof(SocketAddress));
  EXPECT_EQ(cacheLineSize, sizeof(Buffer));

  EXPECT_EQ(24, sizeof(MultiplexedSocket));
  EXPECT_EQ(64, sizeof(ConnectedTCPSocket));
}
