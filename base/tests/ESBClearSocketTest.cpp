#ifndef ESB_SOCKET_TEST_H
#include "ESBSocketTest.h"
#endif

#ifndef ESB_CLEAR_SOCKET_H
#include <ESBClearSocket.h>
#endif

using namespace ESB;

class ClearSocketTest : public SocketTest {};

TEST_F(ClearSocketTest, EchoMessage) {
  ClearSocket client(_server.clearAddress(), "Test", true);

  Error error = client.connect();
  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_TRUE(client.connected());
  ASSERT_FALSE(client.secure());
  ASSERT_TRUE(client.isBlocking());

  for (int i = 0; i < 42; ++i) {
    SSize result = client.send(_message, sizeof(_message));
    ASSERT_EQ(result, sizeof(_message));

    char buffer[sizeof(_message)];
    result = client.receive(buffer, sizeof(buffer));
    ASSERT_EQ(result, sizeof(buffer));
    ASSERT_TRUE(0 == strcmp(_message, buffer));
  }
}
