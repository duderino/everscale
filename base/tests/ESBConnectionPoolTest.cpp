#ifndef ESB_SOCKET_TEST_H
#include "ESBSocketTest.h"
#endif

#ifndef ESB_CONNECTION_POOL_H
#include <ESBConnectionPool.h>
#endif

#ifndef ESB_CLIENT_TLS_SOCKET_H
#include <ESBClientTLSSocket.h>
#endif

using namespace ESB;

class ConnectionPoolTest : public SocketTest {
 public:
  ConnectionPoolTest() : SocketTest(SystemAllocator::Instance()), _clientContexts(42, 3, SystemAllocator::Instance()) {}
  virtual ~ConnectionPoolTest() {}

  virtual void SetUp() {
    TLSContext::Params params;

    Error error = _server.contextIndex().indexDefaultContext(
        params.privateKeyPath("server.key").certificatePath("server.crt").verifyPeerCertificate(false));
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot initialize default server TLS context");
      exit(error);
    }

    error = _clientContexts.indexDefaultContext(params.reset().caCertificatePath("ca.crt").verifyPeerCertificate(true));
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot initialize default client TLS context");
      exit(error);
    }

    error = _clientContexts.indexContext(params.reset()
                                             .privateKeyPath("client.key")
                                             .certificatePath("client.crt")
                                             .caCertificatePath("ca.crt")
                                             .verifyPeerCertificate(true));
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot initialize client mTLS context");
      exit(error);
    }

    SocketTest::SetUp();
  }

  virtual void TearDown() { SocketTest::TearDown(); }

 protected:
  void destroyConnection(ConnectedSocket *connection) {
    connection->~ConnectedSocket();
    SystemAllocator::Instance().deallocate(connection);
  }

  void useConnection(ConnectedSocket *connection) {
    connection->setBlocking(true);
    SSize result = connection->send(_message, sizeof(_message));
    ASSERT_EQ(result, sizeof(_message));

    char buffer[sizeof(_message)];
    result = connection->receive(buffer, sizeof(buffer));
    ASSERT_EQ(result, sizeof(buffer));
    ASSERT_EQ(0, memcmp(_message, buffer, sizeof(_message)));
  }

  ClientTLSContextIndex _clientContexts;

  ESB_DISABLE_AUTO_COPY(ConnectionPoolTest);
};

TEST_F(ConnectionPoolTest, AcquireRelease) {
  ConnectionPool pool("Test", 41, 0, _clientContexts);
  ConnectedSocket *connection = NULL;
  bool reused = false;

  Error error = pool.acquireClearSocket(_server.clearAddress(), &connection, &reused);

  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_EQ(0, pool.hits());
  ASSERT_EQ(1, pool.misses());
  ASSERT_TRUE(connection->connected());
  ASSERT_EQ(0, pool.size());
  ASSERT_FALSE(reused);

  useConnection(connection);

  for (int i = 0; i < 42; ++i) {
    pool.release(connection);
    ASSERT_EQ(1, pool.size());

    error = pool.acquireClearSocket(_server.clearAddress(), &connection, &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_EQ(i + 1, pool.hits());
    ASSERT_EQ(1, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(0, pool.size());
    ASSERT_TRUE(reused);

    useConnection(connection);
  }

  destroyConnection(connection);
}

TEST_F(ConnectionPoolTest, AcquireCloseRelease) {
  ConnectionPool pool("Test", 41, 0, _clientContexts);
  ConnectedSocket *connection = NULL;
  bool reused = true;

  Error error = pool.acquireClearSocket(_server.clearAddress(), &connection, &reused);

  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_EQ(0, pool.hits());
  ASSERT_EQ(1, pool.misses());
  ASSERT_TRUE(connection->connected());
  ASSERT_EQ(0, pool.size());
  ASSERT_FALSE(reused);

  useConnection(connection);

  for (int i = 0; i < 42; ++i) {
    connection->close();
    pool.release(connection);
    ASSERT_EQ(0, pool.size());

    error = pool.acquireClearSocket(_server.clearAddress(), &connection, &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_EQ(0, pool.hits());
    ASSERT_EQ(1 + i + 1, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(0, pool.size());
    ASSERT_FALSE(reused);

    useConnection(connection);
  }

  destroyConnection(connection);
}

TEST_F(ConnectionPoolTest, SecureAcquireRelease) {
  ConnectionPool pool("Test", 41, 0, _clientContexts);
  ConnectedSocket *connection = NULL;
  bool reused = false;

  Error error = pool.acquireTLSSocket("test.server.everscale.com", _server.secureAddress(), &connection, &reused);

  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_EQ(0, pool.hits());
  ASSERT_EQ(1, pool.misses());
  ASSERT_TRUE(connection->connected());
  ASSERT_EQ(0, pool.size());
  ASSERT_FALSE(reused);

  useConnection(connection);

  for (int i = 0; i < 42; ++i) {
    pool.release(connection);
    ASSERT_EQ(1, pool.size());

    error = pool.acquireTLSSocket("test.server.everscale.com", _server.secureAddress(), &connection, &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_EQ(i + 1, pool.hits());
    ASSERT_EQ(1, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(0, pool.size());
    ASSERT_TRUE(reused);

    useConnection(connection);
  }

  destroyConnection(connection);
}

TEST_F(ConnectionPoolTest, SecureAcquireCloseRelease) {
  ConnectionPool pool("Test", 41, 0, _clientContexts);
  ConnectedSocket *connection = NULL;
  bool reused = false;

  Error error = pool.acquireTLSSocket("test.server.everscale.com", _server.secureAddress(), &connection, &reused);

  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_EQ(0, pool.hits());
  ASSERT_EQ(1, pool.misses());
  ASSERT_TRUE(connection->connected());
  ASSERT_EQ(0, pool.size());
  ASSERT_FALSE(reused);

  useConnection(connection);

  for (int i = 0; i < 42; ++i) {
    connection->close();
    pool.release(connection);
    ASSERT_EQ(0, pool.size());

    error = pool.acquireTLSSocket("test.server.everscale.com", _server.secureAddress(), &connection, &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_EQ(0, pool.hits());
    ASSERT_EQ(1 + i + 1, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(0, pool.size());
    ASSERT_FALSE(reused);

    useConnection(connection);
  }

  destroyConnection(connection);
}

TEST_F(ConnectionPoolTest, DistinctTransport) {
  ConnectionPool pool("Test", 41, 0, _clientContexts);
  ConnectedSocket *connection = NULL;
  bool reused = false;
  EmbeddedList connections;
  const char *fqdn = "test.server.everscale.com";

  for (int i = 0; i < 42; ++i) {
    Error error = i % 2 ? pool.acquireClearSocket(_server.clearAddress(), &connection, &reused)
                        : pool.acquireTLSSocket(fqdn, _server.secureAddress(), &connection, &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_EQ(0, pool.hits());
    ASSERT_EQ(i + 1, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(0, pool.size());
    ASSERT_FALSE(reused);
    if (i % 2) {
      ASSERT_FALSE(connection->secure());
    } else {
      ASSERT_TRUE(connection->secure());
    }

    useConnection(connection);
    connections.addLast(connection);
  }

  for (int i = 0; i < 42; ++i) {
    connection = (ConnectedSocket *)connections.removeFirst();
    pool.release(connection);
    ASSERT_EQ(i + 1, pool.size());
  }

  for (int i = 0; i < 42 / 2; ++i) {
    Error error = pool.acquireTLSSocket(fqdn, _server.secureAddress(), &connection, &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_EQ(SocketAddress::TLS, connection->peerAddress().type());
    ASSERT_EQ(i + 1, pool.hits());
    ASSERT_EQ(42, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(42 - i - 1, pool.size());
    ASSERT_TRUE(reused);
    ASSERT_TRUE(connection->secure());

    useConnection(connection);
    destroyConnection(connection);
  }

  {
    Error error = pool.acquireTLSSocket(fqdn, _server.secureAddress(), &connection, &reused);
    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_EQ(SocketAddress::TLS, connection->peerAddress().type());
    ASSERT_EQ(21, pool.hits());
    ASSERT_EQ(42 + 1, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(21, pool.size());
    ASSERT_FALSE(reused);
    ASSERT_TRUE(connection->secure());

    useConnection(connection);
    destroyConnection(connection);
  }

  for (int i = 0; i < 42 / 2; ++i) {
    Error error = pool.acquireClearSocket(_server.clearAddress(), &connection, &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_EQ(SocketAddress::TCP, connection->peerAddress().type());
    ASSERT_EQ(i + 1 + 21, pool.hits());
    ASSERT_EQ(42 + 1, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(21 - i - 1, pool.size());
    ASSERT_TRUE(reused);
    ASSERT_FALSE(connection->secure());

    useConnection(connection);
    destroyConnection(connection);
  }
}

TEST_F(ConnectionPoolTest, CleanupLiveConnections) {
  ConnectionPool pool("Test", 41, 0, _clientContexts);
  ConnectedSocket *connection = NULL;
  bool reused = false;
  EmbeddedList connections;
  const char *fqdn = "test.server.everscale.com";

  for (int i = 0; i < 42; ++i) {
    Error error = i % 2 ? pool.acquireClearSocket(_server.clearAddress(), &connection, &reused)
                        : pool.acquireTLSSocket(fqdn, _server.secureAddress(), &connection, &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_TRUE(connection->connected());
    ASSERT_FALSE(reused);
    if (i % 2) {
      ASSERT_FALSE(connection->secure());
    } else {
      ASSERT_TRUE(connection->secure());
    }

    useConnection(connection);
    connections.addLast(connection);
  }

  for (int i = 0; i < 42; ++i) {
    connection = (ConnectedSocket *)connections.removeFirst();
    pool.release(connection);
    ASSERT_EQ(i + 1, pool.size());
  }

  // Let pool go out of scope with lots of active connections, LSAN should say no leaks
}

TEST_F(ConnectionPoolTest, CleanupDeadConnections) {
  ConnectionPool pool("Test", 41, 0, _clientContexts);
  ConnectedSocket *connection = NULL;
  bool reused = false;
  EmbeddedList connections;
  const char *fqdn = "test.server.everscale.com";

  for (int i = 0; i < 42; ++i) {
    Error error = i % 2 ? pool.acquireClearSocket(_server.clearAddress(), &connection, &reused)
                        : pool.acquireTLSSocket(fqdn, _server.secureAddress(), &connection, &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_TRUE(connection->connected());
    ASSERT_FALSE(reused);
    if (i % 2) {
      ASSERT_FALSE(connection->secure());
    } else {
      ASSERT_TRUE(connection->secure());
    }

    useConnection(connection);
    connections.addLast(connection);
  }

  for (int i = 0; i < 42; ++i) {
    connection = (ConnectedSocket *)connections.removeFirst();
    connection->close();
    pool.release(connection);
    ASSERT_EQ(0, pool.size());
  }

  // Let pool go out of scope with lots of dead connections, LSAN should say no leaks
}

TEST_F(ConnectionPoolTest, MostSpecificClientCertificate) {
  ConnectionPool pool("Test", 41, 3, _clientContexts);
  TLSContextPointer defaultContext = _clientContexts.defaultContext();
  TLSContextPointer san3Context;
  TLSContextPointer san4Context;

  //
  // Register client certificates
  //

  {
    // san3 has:
    //
    //  [alt_names]
    //  DNS.1 = f*.server.everscale.com
    //  DNS.2 = *z.server.everscale.com
    //  DNS.3 = b*r.server.everscale.com
    //  IP.1 = 1.2.3.4
    //  IP.2 = 5.6.7.8
    //
    TLSContext::Params params;
    ASSERT_EQ(ESB_SUCCESS, _clientContexts.indexContext(params.reset()
                                                            .privateKeyPath("san3.key")
                                                            .certificatePath("san3.crt")
                                                            .caCertificatePath("ca.crt")
                                                            .verifyPeerCertificate(true),
                                                        &san3Context));
    ASSERT_FALSE(san3Context.isNull());

    // san4 has:
    //
    //    [alt_names]
    //    DNS.1 = q*ux.server.everscale.com
    //    DNS.2 = c*.server.everscale.com
    //    DNS.3 = *ly.server.everscale.com
    //
    ASSERT_EQ(ESB_SUCCESS, _clientContexts.indexContext(params.reset()
                                                            .privateKeyPath("san4.key")
                                                            .certificatePath("san4.crt")
                                                            .caCertificatePath("ca.crt")
                                                            .verifyPeerCertificate(true),
                                                        &san4Context));
    ASSERT_FALSE(san4Context.isNull());
  }

  //
  // Add a connection for each client certificate, including the default, to the connection pool
  //

  {
    ClientTLSSocket *connection = NULL;
    bool reused = false;

    // matches nothing, so should be the default certificate
    Error error = pool.acquireTLSSocket("default.server.everscale.com", _server.secureAddress(),
                                        (ConnectedSocket **)&connection, &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_FALSE(reused);
    ASSERT_EQ(defaultContext->rawContext(), connection->context()->rawContext());
    ASSERT_EQ(0, pool.hits());
    ASSERT_EQ(1, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(0, pool.size());

    useConnection(connection);
    pool.release(connection);
    ASSERT_EQ(1, pool.size());

    // matches san3 which is more specific than the default certificate
    error = pool.acquireTLSSocket("foo.server.everscale.com", _server.secureAddress(), (ConnectedSocket **)&connection,
                                  &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_FALSE(reused);
    ASSERT_EQ(san3Context->rawContext(), connection->context()->rawContext());
    ASSERT_EQ(0, pool.hits());
    ASSERT_EQ(2, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(1, pool.size());

    useConnection(connection);
    pool.release(connection);
    ASSERT_EQ(2, pool.size());

    // matches san4 which is more specific than the default certificate
    error = pool.acquireTLSSocket("qux.server.everscale.com", _server.secureAddress(), (ConnectedSocket **)&connection,
                                  &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_FALSE(reused);
    ASSERT_EQ(san4Context->rawContext(), connection->context()->rawContext());
    ASSERT_EQ(0, pool.hits());
    ASSERT_EQ(3, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(2, pool.size());

    useConnection(connection);
    pool.release(connection);
    ASSERT_EQ(3, pool.size());
  }

  //
  // Borrow connections from connection pool, verifying the most specific client certificate is always selected
  //

  {
    ClientTLSSocket *connection = NULL;
    bool reused = false;

    // matches nothing, so should be the default certificate
    Error error = pool.acquireTLSSocket("default.server.everscale.com", _server.secureAddress(),
                                        (ConnectedSocket **)&connection, &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_TRUE(reused);
    ASSERT_EQ(defaultContext->rawContext(), connection->context()->rawContext());
    ASSERT_EQ(1, pool.hits());
    ASSERT_EQ(3, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(2, pool.size());

    useConnection(connection);
    pool.release(connection);
    ASSERT_EQ(3, pool.size());

    // matches san3 which is more specific than the default certificate
    error = pool.acquireTLSSocket("foo.server.everscale.com", _server.secureAddress(), (ConnectedSocket **)&connection,
                                  &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_TRUE(reused);
    ASSERT_EQ(san3Context->rawContext(), connection->context()->rawContext());
    ASSERT_EQ(2, pool.hits());
    ASSERT_EQ(3, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(2, pool.size());

    useConnection(connection);
    pool.release(connection);
    ASSERT_EQ(3, pool.size());

    // matches san4 which is more specific than the default certificate
    error = pool.acquireTLSSocket("qux.server.everscale.com", _server.secureAddress(), (ConnectedSocket **)&connection,
                                  &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_TRUE(reused);
    ASSERT_EQ(san4Context->rawContext(), connection->context()->rawContext());
    ASSERT_EQ(3, pool.hits());
    ASSERT_EQ(3, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(2, pool.size());

    useConnection(connection);
    pool.release(connection);
    ASSERT_EQ(3, pool.size());
  }
}

TEST_F(ConnectionPoolTest, CompatibleServerCertificate) {
  ConnectionPool pool("Test", 41, 3, _clientContexts);
  TLSContextPointer san3Context;
  TLSContextPointer san4Context;

  //
  // Register client certificates
  //

  {
    // san3 has:
    //
    //  [alt_names]
    //  DNS.1 = f*.server.everscale.com
    //  DNS.2 = *z.server.everscale.com
    //  DNS.3 = b*r.server.everscale.com
    //  IP.1 = 1.2.3.4
    //  IP.2 = 5.6.7.8
    //
    TLSContext::Params params;
    ASSERT_EQ(ESB_SUCCESS, _clientContexts.indexContext(params.reset()
                                                            .privateKeyPath("san3.key")
                                                            .certificatePath("san3.crt")
                                                            .caCertificatePath("ca.crt")
                                                            .verifyPeerCertificate(true),
                                                        &san3Context));
    ASSERT_FALSE(san3Context.isNull());

    // san4 has:
    //
    //    [alt_names]
    //    DNS.1 = q*ux.server.everscale.com
    //    DNS.2 = c*.server.everscale.com
    //    DNS.3 = *ly.server.everscale.com
    //
    ASSERT_EQ(ESB_SUCCESS, _clientContexts.indexContext(params.reset()
                                                            .privateKeyPath("san4.key")
                                                            .certificatePath("san4.crt")
                                                            .caCertificatePath("ca.crt")
                                                            .verifyPeerCertificate(true),
                                                        &san4Context));
    ASSERT_FALSE(san4Context.isNull());
  }

  //
  // Make default server cert more restrictive
  //

  {
    // san5 has:
    //
    //    [alt_names]
    //    DNS.1 = foo.server.everscale.com
    //    DNS.2 = bar.server.everscale.com
    //    DNS.3 = baz.server.everscale.com
    //    DNS.4 = q*ux.server.everscale.com
    //
    _server.contextIndex().clear();
    TLSContext::Params params;
    ASSERT_EQ(ESB_SUCCESS, _server.contextIndex().indexDefaultContext(params.reset()
                                                                          .privateKeyPath("san5.key")
                                                                          .certificatePath("san5.crt")
                                                                          .caCertificatePath("ca.crt")
                                                                          .verifyPeerCertificate(true)));
    ASSERT_FALSE(_server.contextIndex().defaultContext().isNull());
  }

  //
  // Add a connection for each client certificate to the connection pool
  //

  {
    ClientTLSSocket *connection = NULL;
    bool reused = false;

    // matches san3's f*.server.everscale.com
    Error error = pool.acquireTLSSocket("bar.server.everscale.com", _server.secureAddress(),
                                        (ConnectedSocket **)&connection, &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_FALSE(reused);
    ASSERT_EQ(san3Context->rawContext(), connection->context()->rawContext());
    ASSERT_EQ(0, pool.hits());
    ASSERT_EQ(1, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(0, pool.size());

    useConnection(connection);
    pool.release(connection);
    ASSERT_EQ(1, pool.size());

    // matches san4's b*z.server.everscale.com which is more specific than san3's *z.server.everscale.com
    error = pool.acquireTLSSocket("baz.server.everscale.com", _server.secureAddress(), (ConnectedSocket **)&connection,
                                  &reused);

    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_FALSE(reused);
    ASSERT_EQ(san4Context->rawContext(), connection->context()->rawContext());
    ASSERT_EQ(0, pool.hits());
    ASSERT_EQ(2, pool.misses());
    ASSERT_TRUE(connection->connected());
    ASSERT_EQ(1, pool.size());

    useConnection(connection);
    pool.release(connection);
    ASSERT_EQ(2, pool.size());
  }

  //
  // Borrow connections from connection pool, verifying the most specific client certificate and a compatible server
  // certificate is always selected
  //

  ClientTLSSocket *connection = NULL;
  bool reused = false;

  // foo.server.everscale.com matches the san3 client cert's f*.server.everscale.com and the server cert's
  // foo.server.everscale.com
  Error error = pool.acquireTLSSocket("foo.server.everscale.com", _server.secureAddress(),
                                      (ConnectedSocket **)&connection, &reused);

  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_TRUE(reused);
  ASSERT_EQ(san3Context->rawContext(), connection->context()->rawContext());
  ASSERT_EQ(1, pool.hits());
  ASSERT_EQ(2, pool.misses());
  ASSERT_TRUE(connection->connected());
  ASSERT_EQ(1, pool.size());

  useConnection(connection);
  pool.release(connection);
  ASSERT_EQ(2, pool.size());

  // qux.server.everscale.com matches san4 client cert's q*ux.server.everscale.com and is compatible with server cert's
  // q*ux.server.everscale.com
  error = pool.acquireTLSSocket("qux.server.everscale.com", _server.secureAddress(), (ConnectedSocket **)&connection,
                                &reused);

  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_TRUE(reused);
  ASSERT_EQ(san4Context->rawContext(), connection->context()->rawContext());
  ASSERT_EQ(2, pool.hits());
  ASSERT_EQ(2, pool.misses());
  ASSERT_TRUE(connection->connected());
  ASSERT_EQ(1, pool.size());

  useConnection(connection);
  pool.release(connection);
  ASSERT_EQ(2, pool.size());

  // corge.server.everscale.com matches san4 client cert's c*.server.everscale.com but is not compatible with any server
  // cert, so a new connection will be created that will fail the handshake
  error = pool.acquireTLSSocket("corge.server.everscale.com", _server.secureAddress(), (ConnectedSocket **)&connection,
                                &reused);

  // A new TLS client connection with client context san4 will be created
  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_FALSE(reused);
  ASSERT_EQ(san4Context->rawContext(), connection->context()->rawContext());
  ASSERT_EQ(2, pool.hits());
  ASSERT_EQ(3, pool.misses());
  ASSERT_TRUE(connection->connected());
  ASSERT_EQ(2, pool.size());

  // But the handshake will fail
  connection->setBlocking(true);
  SSize result = connection->send(_message, sizeof(_message));
  ASSERT_EQ(-1, result);
  ASSERT_EQ(ESB_TLS_HANDSHAKE_ERROR, LastError());

  // And the connection cannot be returned to the connection pool
  ASSERT_EQ(2, pool.size());
  pool.release(connection);
  ASSERT_EQ(2, pool.size());

  // Load a server cert that is compatible with corge.server.everscale.com

  // san6 has:
  //
  //  [alt_names]
  //  DNS.1 = c*ge.server.everscale.com
  //
  TLSContext::Params params;
  ASSERT_EQ(ESB_SUCCESS, _server.contextIndex().indexContext(params.reset()
                                                                 .privateKeyPath("san6.key")
                                                                 .certificatePath("san6.crt")
                                                                 .caCertificatePath("ca.crt")
                                                                 .verifyPeerCertificate(true)));

  // corge.server.everscale.com matches san4 client cert's c*.server.everscale.com and is now compatible with the san6
  // server cert's c*.server.everscale.com
  error = pool.acquireTLSSocket("corge.server.everscale.com", _server.secureAddress(), (ConnectedSocket **)&connection,
                                &reused);

  // A new TLS client connection with client context san4 will be created
  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_FALSE(reused);
  ASSERT_EQ(san4Context->rawContext(), connection->context()->rawContext());
  ASSERT_EQ(2, pool.hits());
  ASSERT_EQ(4, pool.misses());
  ASSERT_TRUE(connection->connected());
  ASSERT_EQ(2, pool.size());

  useConnection(connection);
  pool.release(connection);
  ASSERT_EQ(3, pool.size());

  // And finally, corge.server.everscale.com connections can be reused
  error = pool.acquireTLSSocket("corge.server.everscale.com", _server.secureAddress(), (ConnectedSocket **)&connection,
                                &reused);

  // A new TLS client connection with client context san4 will be created
  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_TRUE(reused);
  ASSERT_EQ(san4Context->rawContext(), connection->context()->rawContext());
  ASSERT_EQ(3, pool.hits());
  ASSERT_EQ(4, pool.misses());
  ASSERT_TRUE(connection->connected());
  ASSERT_EQ(2, pool.size());

  useConnection(connection);
  pool.release(connection);
  ASSERT_EQ(3, pool.size());
}
