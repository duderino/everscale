#ifndef ESB_CONNECTION_POOL_H
#include <ESBConnectionPool.h>
#endif

#ifndef ESB_LISTENING_SOCKET_H
#include <ESBListeningSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

class ConnectionPoolTest : public ::testing::Test {
 public:
  ConnectionPoolTest()
      : _clearPeerAddress("127.0.0.1", 0, SocketAddress::TCP),
        _securePeerAddress("127.0.0.1", 0, SocketAddress::TLS),
        _clearListener("clear", _clearPeerAddress, 42),
        _secureListener("secure", _securePeerAddress, 42) {}

  virtual void SetUp() {
    Error error = _clearListener.bind();
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "[%s] cannot bind to ephemeral port", _clearListener.name());
      assert(!"Cannot bind to ephemeral port");
      return;
    }

    error = _secureListener.bind();
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "[%s] cannot bind to ephemeral port", _secureListener.name());
      assert(!"Cannot bind to ephemeral port");
      return;
    }

    error = _clearListener.listen();
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "[%s] cannot listen on ephemeral port", _clearListener.name());
      assert(!"Cannot listen on ephemeral port");
      return;
    }

    error = _secureListener.listen();
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "[%s] cannot listen on ephemeral port", _secureListener.name());
      assert(!"Cannot listen on ephemeral port");
      return;
    }

    _clearPeerAddress.setPort(_clearListener.listeningAddress().port());
    _securePeerAddress.setPort(_secureListener.listeningAddress().port());
  }

 protected:
  SocketAddress _clearPeerAddress;
  SocketAddress _securePeerAddress;

 private:
  ListeningSocket _clearListener;
  ListeningSocket _secureListener;
};

static void DestroyConnection(ConnectedSocket *connection) {
  connection->~ConnectedSocket();
  SystemAllocator::Instance().deallocate(connection);
}

TEST_F(ConnectionPoolTest, AcquireRelease) {
  ConnectionPool pool("Testy", "McTest", 42, 0);
  ConnectedSocket *connection = NULL;
  bool reused = false;

  Error error = pool.acquire(_clearPeerAddress, &connection, &reused);

  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, pool.hits());
  EXPECT_EQ(1, pool.misses());
  EXPECT_EQ(true, connection->connected());
  EXPECT_EQ(0, pool.size());
  EXPECT_EQ(false, reused);

  for (int i = 0; i < 42; ++i) {
    pool.release(connection);
    EXPECT_EQ(1, pool.size());

    error = pool.acquire(_clearPeerAddress, &connection, &reused);

    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(i + 1, pool.hits());
    EXPECT_EQ(1, pool.misses());
    EXPECT_EQ(true, connection->connected());
    EXPECT_EQ(0, pool.size());
    EXPECT_EQ(true, reused);
  }

  DestroyConnection(connection);
}

TEST_F(ConnectionPoolTest, AcquireCloseRelease) {
  ConnectionPool pool("Testy", "McTest", 42, 0);
  ConnectedSocket *connection = NULL;
  bool reused = true;

  Error error = pool.acquire(_clearPeerAddress, &connection, &reused);

  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, pool.hits());
  EXPECT_EQ(1, pool.misses());
  EXPECT_EQ(true, connection->connected());
  EXPECT_EQ(0, pool.size());
  EXPECT_EQ(false, reused);

  for (int i = 0; i < 42; ++i) {
    connection->close();
    pool.release(connection);
    EXPECT_EQ(0, pool.size());

    error = pool.acquire(_clearPeerAddress, &connection, &reused);

    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(0, pool.hits());
    EXPECT_EQ(1 + i + 1, pool.misses());
    EXPECT_EQ(true, connection->connected());
    EXPECT_EQ(0, pool.size());
    EXPECT_EQ(false, reused);
  }

  DestroyConnection(connection);
}

TEST_F(ConnectionPoolTest, DistinctTransport) {
  ConnectionPool pool("Testy", "McTest", 42, 0);
  ConnectedSocket *connection = NULL;
  bool reused = false;
  EmbeddedList connections;

  for (int i = 0; i < 42; ++i) {
    Error error = i % 2 ? pool.acquire(_clearPeerAddress, &connection, &reused)
                        : pool.acquire(_securePeerAddress, &connection, &reused);

    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(0, pool.hits());
    EXPECT_EQ(i + 1, pool.misses());
    EXPECT_EQ(true, connection->connected());
    EXPECT_EQ(0, pool.size());
    EXPECT_EQ(false, reused);

    connections.addLast(connection);
  }

  for (int i = 0; i < 42; ++i) {
    connection = (ConnectedSocket *)connections.removeFirst();
    pool.release(connection);
    EXPECT_EQ(i + 1, pool.size());
  }

  for (int i = 0; i < 42 / 2; ++i) {
    Error error = pool.acquire(_securePeerAddress, &connection, &reused);

    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(SocketAddress::TLS, connection->peerAddress().type());
    EXPECT_EQ(i + 1, pool.hits());
    EXPECT_EQ(42, pool.misses());
    EXPECT_EQ(true, connection->connected());
    EXPECT_EQ(42 - i - 1, pool.size());
    EXPECT_EQ(true, reused);

    DestroyConnection(connection);
  }

  {
    Error error = pool.acquire(_securePeerAddress, &connection, &reused);
    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(SocketAddress::TLS, connection->peerAddress().type());
    EXPECT_EQ(21, pool.hits());
    EXPECT_EQ(42 + 1, pool.misses());
    EXPECT_EQ(true, connection->connected());
    EXPECT_EQ(21, pool.size());
    EXPECT_EQ(false, reused);

    DestroyConnection(connection);
  }

  for (int i = 0; i < 42 / 2; ++i) {
    Error error = pool.acquire(_clearPeerAddress, &connection, &reused);

    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(SocketAddress::TCP, connection->peerAddress().type());
    EXPECT_EQ(i + 1 + 21, pool.hits());
    EXPECT_EQ(42 + 1, pool.misses());
    EXPECT_EQ(true, connection->connected());
    EXPECT_EQ(21 - i - 1, pool.size());
    EXPECT_EQ(true, reused);

    DestroyConnection(connection);
  }
}

TEST_F(ConnectionPoolTest, CleanupLiveConnections) {
  ConnectionPool pool("Testy", "McTest", 42, 0);
  ConnectedSocket *connection = NULL;
  bool reused = false;
  EmbeddedList connections;

  for (int i = 0; i < 42; ++i) {
    Error error = i % 2 ? pool.acquire(_clearPeerAddress, &connection, &reused)
                        : pool.acquire(_securePeerAddress, &connection, &reused);

    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(true, connection->connected());
    EXPECT_EQ(false, reused);

    connections.addLast(connection);
  }

  for (int i = 0; i < 42; ++i) {
    connection = (ConnectedSocket *)connections.removeFirst();
    pool.release(connection);
    EXPECT_EQ(i + 1, pool.size());
  }

  // Let pool go out of scope with lots of active connections, ASAN should say no leaks
}

TEST_F(ConnectionPoolTest, CleanupDeadConnections) {
  ConnectionPool pool("Testy", "McTest", 42, 0);
  ConnectedSocket *connection = NULL;
  bool reused = false;
  EmbeddedList connections;

  for (int i = 0; i < 42; ++i) {
    Error error = i % 2 ? pool.acquire(_clearPeerAddress, &connection, &reused)
                        : pool.acquire(_securePeerAddress, &connection, &reused);

    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(true, connection->connected());
    EXPECT_EQ(false, reused);

    connections.addLast(connection);
  }

  for (int i = 0; i < 42; ++i) {
    connection = (ConnectedSocket *)connections.removeFirst();
    connection->close();
    pool.release(connection);
    EXPECT_EQ(0, pool.size());
  }

  // Let pool go out of scope with lots of dead connections, ASAN should say no leaks
}
