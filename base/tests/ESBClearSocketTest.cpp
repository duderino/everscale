#ifndef ESB_SOCKET_TEST_H
#include "ESBSocketTest.h"
#endif

#ifndef ESB_CLEAR_SOCKET_H
#include <ESBClearSocket.h>
#endif

using namespace ESB;

class ClearSocketTest : public SocketTest {};

TEST_F(ClearSocketTest, EchoMessage) {
  ClearSocket client(_clearListenerAddress, "Test", true);

  Error error = client.connect();
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_TRUE(client.connected());
  EXPECT_FALSE(client.secure());
  EXPECT_TRUE(client.isBlocking());

  Socket::State state;
  error = _clearListener.accept(&state);
  EXPECT_EQ(ESB_SUCCESS, error);

  ClearSocket server(state, "Test");
  EXPECT_TRUE(server.connected());
  EXPECT_FALSE(server.secure());
  EXPECT_FALSE(server.isBlocking());

  SSize result = client.send(_message, sizeof(_message));
  EXPECT_EQ(result, sizeof(_message));

  char buffer[sizeof(_message)];
  result = server.receive(buffer, sizeof(buffer));
  EXPECT_EQ(result, sizeof(buffer));
  EXPECT_TRUE(0 == strcmp(_message, buffer));

  result = server.send(buffer, sizeof(buffer));
  EXPECT_EQ(result, sizeof(buffer));

  result = client.receive(buffer, sizeof(buffer));
  EXPECT_EQ(result, sizeof(buffer));
  EXPECT_TRUE(0 == strcmp(_message, buffer));
}
