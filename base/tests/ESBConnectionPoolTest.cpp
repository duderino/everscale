#ifndef ESB_SOCKET_TEST_H
#include "ESBSocketTest.h"
#endif

#ifndef ESB_CONNECTION_POOL_H
#include <ESBConnectionPool.h>
#endif

using namespace ESB;

class ConnectionPoolTest : public SocketTest {
};

static void DestroyConnection(ConnectedSocket *connection) {
  connection->~ConnectedSocket();
  SystemAllocator::Instance().deallocate(connection);
}

TEST_F(ConnectionPoolTest, AcquireRelease) {
  ConnectionPool pool("Test", 41, 0);
  ConnectedSocket *connection = NULL;
  bool reused = false;

  Error error = pool.acquireClearSocket(_clearListenerAddress, &connection, &reused);

  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, pool.hits());
  EXPECT_EQ(1, pool.misses());
  EXPECT_TRUE(connection->connected());
  EXPECT_EQ(0, pool.size());
  EXPECT_FALSE(reused);

  for (int i = 0; i < 42; ++i) {
    pool.release(connection);
    EXPECT_EQ(1, pool.size());

    error = pool.acquireClearSocket(_clearListenerAddress, &connection, &reused);

    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(i + 1, pool.hits());
    EXPECT_EQ(1, pool.misses());
    EXPECT_TRUE(connection->connected());
    EXPECT_EQ(0, pool.size());
    EXPECT_TRUE(reused);
  }

  DestroyConnection(connection);
}

TEST_F(ConnectionPoolTest, AcquireCloseRelease) {
  ConnectionPool pool("Test", 41, 0);
  ConnectedSocket *connection = NULL;
  bool reused = true;

  Error error = pool.acquireClearSocket(_clearListenerAddress, &connection, &reused);

  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, pool.hits());
  EXPECT_EQ(1, pool.misses());
  EXPECT_TRUE(connection->connected());
  EXPECT_EQ(0, pool.size());
  EXPECT_FALSE(reused);

  for (int i = 0; i < 42; ++i) {
    connection->close();
    pool.release(connection);
    EXPECT_EQ(0, pool.size());

    error = pool.acquireClearSocket(_clearListenerAddress, &connection, &reused);

    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(0, pool.hits());
    EXPECT_EQ(1 + i + 1, pool.misses());
    EXPECT_TRUE(connection->connected());
    EXPECT_EQ(0, pool.size());
    EXPECT_FALSE(reused);
  }

  DestroyConnection(connection);
}

TEST_F(ConnectionPoolTest, DistinctTransport) {
  ConnectionPool pool("Test", 41, 0);
  ConnectedSocket *connection = NULL;
  bool reused = false;
  EmbeddedList connections;
  HostAddress secureListenerAddress("server.everscale.com", _secureListenerAddress);

  for (int i = 0; i < 42; ++i) {
    Error error = i % 2 ? pool.acquireClearSocket(_clearListenerAddress, &connection, &reused)
                        : pool.acquireTLSSocket(secureListenerAddress, &connection, &reused);

    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(0, pool.hits());
    EXPECT_EQ(i + 1, pool.misses());
    EXPECT_TRUE(connection->connected());
    EXPECT_EQ(0, pool.size());
    EXPECT_FALSE(reused);
    if (i % 2) {
      EXPECT_FALSE(connection->secure());
    } else {
      EXPECT_TRUE(connection->secure());
    }

    connections.addLast(connection);
  }

  for (int i = 0; i < 42; ++i) {
    connection = (ConnectedSocket *)connections.removeFirst();
    pool.release(connection);
    EXPECT_EQ(i + 1, pool.size());
  }

  for (int i = 0; i < 42 / 2; ++i) {
    Error error = pool.acquireTLSSocket(secureListenerAddress, &connection, &reused);

    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(SocketAddress::TLS, connection->peerAddress().type());
    EXPECT_EQ(i + 1, pool.hits());
    EXPECT_EQ(42, pool.misses());
    EXPECT_TRUE(connection->connected());
    EXPECT_EQ(42 - i - 1, pool.size());
    EXPECT_TRUE(reused);
    EXPECT_TRUE(connection->secure());

    DestroyConnection(connection);
  }

  {
    Error error = pool.acquireTLSSocket(secureListenerAddress, &connection, &reused);
    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(SocketAddress::TLS, connection->peerAddress().type());
    EXPECT_EQ(21, pool.hits());
    EXPECT_EQ(42 + 1, pool.misses());
    EXPECT_TRUE(connection->connected());
    EXPECT_EQ(21, pool.size());
    EXPECT_FALSE(reused);
    EXPECT_TRUE(connection->secure());

    DestroyConnection(connection);
  }

  for (int i = 0; i < 42 / 2; ++i) {
    Error error = pool.acquireClearSocket(_clearListenerAddress, &connection, &reused);

    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_EQ(SocketAddress::TCP, connection->peerAddress().type());
    EXPECT_EQ(i + 1 + 21, pool.hits());
    EXPECT_EQ(42 + 1, pool.misses());
    EXPECT_TRUE(connection->connected());
    EXPECT_EQ(21 - i - 1, pool.size());
    EXPECT_TRUE(reused);
    EXPECT_FALSE(connection->secure());

    DestroyConnection(connection);
  }
}

TEST_F(ConnectionPoolTest, CleanupLiveConnections) {
  ConnectionPool pool("Test", 41, 0);
  ConnectedSocket *connection = NULL;
  bool reused = false;
  EmbeddedList connections;
  HostAddress secureListenerAddress("server.everscale.com", _secureListenerAddress);

  for (int i = 0; i < 42; ++i) {
    Error error = i % 2 ? pool.acquireClearSocket(_clearListenerAddress, &connection, &reused)
                        : pool.acquireTLSSocket(secureListenerAddress, &connection, &reused);

    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_TRUE(connection->connected());
    EXPECT_FALSE(reused);
    if (i % 2) {
      EXPECT_FALSE(connection->secure());
    } else {
      EXPECT_TRUE(connection->secure());
    }

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
  ConnectionPool pool("Test", 41, 0);
  ConnectedSocket *connection = NULL;
  bool reused = false;
  EmbeddedList connections;
  HostAddress secureListenerAddress("server.everscale.com", _secureListenerAddress);

  for (int i = 0; i < 42; ++i) {
    Error error = i % 2 ? pool.acquireClearSocket(_clearListenerAddress, &connection, &reused)
                        : pool.acquireTLSSocket(secureListenerAddress, &connection, &reused);

    EXPECT_EQ(ESB_SUCCESS, error);
    EXPECT_TRUE(connection->connected());
    EXPECT_FALSE(reused);
    if (i % 2) {
      EXPECT_FALSE(connection->secure());
    } else {
      EXPECT_TRUE(connection->secure());
    }

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
