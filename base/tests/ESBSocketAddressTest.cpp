#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#include <gtest/gtest.h>
#include <string.h>

using namespace ESB;

TEST(SocketAddress, PresentationFormat) {
  SocketAddress address("255.255.255.255", 65535,
                        SocketAddress::TransportType::TCP);
  char buffer[ESB_IPV6_PRESENTATION_SIZE];
  address.presentationAddress(buffer, sizeof(buffer));
  EXPECT_TRUE(0 == ::strcmp("255.255.255.255", buffer));
}

TEST(SocketAddress, LogFormat) {
  SocketAddress address("255.255.255.255", 65535,
                        SocketAddress::TransportType::TCP);
  char buffer[ESB_LOG_ADDRESS_SIZE];
  address.logAddress(buffer, sizeof(buffer), 42);
  EXPECT_TRUE(0 == ::strcmp("255.255.255.255:65535,42", buffer));
}
