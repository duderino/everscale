#ifndef ESB_SOCKET_TEST_H
#include "ESBSocketTest.h"
#endif

#ifndef ESB_CLIENT_TLS_SOCKET_H
#include <ESBClientTLSSocket.h>
#endif

#ifndef ESB_SERVER_TLS_SOCKET_H
#include <ESBServerTLSSocket.h>
#endif

#ifndef ESB_THREAD_H
#include <ESBThread.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

class TLSEchoServer : public Thread {
 public:
  TLSEchoServer(UInt32 messageSize, ListeningSocket &listeningSocket) : _messageSize(messageSize), _listeningSocket(listeningSocket) {}
  virtual ~TLSEchoServer() {};

 protected:
  virtual void run() {
    char *message = (char *) malloc(_messageSize);
    assert(message);
    _listeningSocket.setBlocking(true);

    while (isRunning()) {
      Socket::State state;
      ESB::Error error = _listeningSocket.accept(&state);
      assert(ESB_SUCCESS == error);

      ServerTLSSocket server(state, "test");
      assert(server.connected());
      assert(server.secure());
      assert(server.isBlocking());

      while (isRunning()) {
        UInt32 bytesReceived = 0U;
        while (bytesReceived < _messageSize && isRunning() && server.connected()) {
          SSize result = server.receive(message + bytesReceived, _messageSize - bytesReceived);
          if (0 == result) {
            server.close();
            break;
          } else if (0 > result) {
            ESB_LOG_ERROR_ERRNO(LastError(), "[%s] cannot receive data", server.name());
            server.close();
            break;
          }
          bytesReceived += result;
        }

        UInt32 bytesSent = 0U;
        while (bytesSent < _messageSize && isRunning() && server.connected()) {
          SSize result = server.send(message + bytesSent, _messageSize - bytesSent);
          if (0 >= result) {
            server.close();
            ESB_LOG_ERROR_ERRNO(LastError(), "[%s] cannot send data", server.name());
            break;
          }
          bytesSent += result;
        }
      }

      server.close();
    }

    if (message) free(message);
  }

 private:
  UInt32 _messageSize;
  ListeningSocket &_listeningSocket;
};

class TLSSocketTest : public SocketTest {
 public:
  TLSSocketTest() : _serverThread(sizeof(_message), _secureListener) {}
  virtual ~TLSSocketTest() {};

  static void SetUpTestSuite() {
    SocketTest::SetUpTestSuite();

    int maxVerifyDepth = 42;
    const char *caPath = "tests/ca.crt";
    const char *certPath = "tests/server.crt";
    const char *keyPath = "tests/server.key";

    {
      char cwd[ESB_MAX_PATH];
      if (!getcwd(cwd, sizeof(cwd))) {
        ESB_LOG_WARNING_ERRNO(LastError(),
                              "Cannot determine current working directory");
      } else {
        ESB_LOG_DEBUG("Current working dir: %s", cwd);
      }
    }

    Error error = ClientTLSSocket::Initialize(caPath, maxVerifyDepth);
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot initialize client TLS support");
      exit(error);
    }

    error = ServerTLSSocket::Initialize(keyPath, certPath);
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot initialize server TLS support");
      exit(error);
    }
  }

  static void TearDownTestSuite() {
    SocketTest::TearDownTestSuite();
  }

  virtual void SetUp() {
    SocketTest::SetUp();
    Error error = _serverThread.start();
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot start background server thread");
      exit(error);
    }
  }

  virtual void TearDown() {
    _serverThread.stop();
    Error error = _serverThread.join();
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot join background server thread");
      exit(error);
    }
    SocketTest::TearDown();
  }

 private:
  TLSEchoServer _serverThread;
};



TEST_F(TLSSocketTest, EchoMessage) {
  HostAddress serverAddress("test.server.everscale.com", _secureListenerAddress);
  ClientTLSSocket client(serverAddress, "test", true);

  Error error = client.connect();
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_TRUE(client.connected());
  EXPECT_TRUE(client.secure());
  EXPECT_TRUE(client.isBlocking());

  SSize result = client.send(_message, sizeof(_message));
  EXPECT_EQ(result, sizeof(_message));

  char buffer[sizeof(_message)];
  result = client.receive(buffer, sizeof(buffer));
  EXPECT_EQ(result, sizeof(buffer));

  EXPECT_TRUE(0 == strcmp(_message, buffer));

  client.close();
}