#ifndef ESB_SOCKET_TEST_H
#include "ESBSocketTest.h"
#endif

#ifndef ESB_CLIENT_TLS_SOCKET_H
#include <ESBClientTLSSocket.h>
#endif

#ifndef ESB_CLIENT_TLS_CONTEXT_INDEX_H
#include <ESBClientTLSContextIndex.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

class TLSSocketTest : public SocketTest {
 public:
  TLSSocketTest() : SocketTest(SystemAllocator::Instance()), _clientContexts(42, 3, SystemAllocator::Instance()) {}
  virtual ~TLSSocketTest(){};

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
                                             .verifyPeerCertificate(true),
                                         &_clientMutualContext);
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot initialize client mTLS context");
      exit(error);
    }

    SocketTest::SetUp();
  }

  virtual void TearDown() { SocketTest::TearDown(); }

 protected:
  ClientTLSContextIndex _clientContexts;
  TLSContextPointer _clientMutualContext;

  ESB_DISABLE_AUTO_COPY(TLSSocketTest);
};

TEST_F(TLSSocketTest, EchoMessage) {
  ClientTLSSocket client("test.server.everscale.com", _server.secureAddress(), "test", _clientContexts.defaultContext(),
                         true);

  Error error = client.connect();
  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_TRUE(client.connected());
  ASSERT_TRUE(client.secure());
  ASSERT_TRUE(client.isBlocking());

  SSize result = client.send(_message, sizeof(_message));
  ASSERT_EQ(result, sizeof(_message));

  char buffer[sizeof(_message)];
  result = client.receive(buffer, sizeof(buffer));
  ASSERT_EQ(result, sizeof(buffer));

  ASSERT_TRUE(0 == strcmp(_message, buffer));
}

TEST_F(TLSSocketTest, HostnameMismatch) {
  ClientTLSSocket client("mismatch.everscale.com", _server.secureAddress(), "test", _clientContexts.defaultContext(),
                         true);

  Error error = client.connect();
  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_TRUE(client.connected());
  ASSERT_TRUE(client.secure());
  ASSERT_TRUE(client.isBlocking());

  SSize result = client.send(_message, sizeof(_message));
  ASSERT_GE(result, -1);
  ASSERT_EQ(ESB_TLS_HANDSHAKE_ERROR, LastError());
}

TEST_F(TLSSocketTest, CNnotSANmatch) {
  // Default cert has CN of server.everscale.com and
  //
  // [alt_names]
  // DNS.1 = *.server.everscale.com
  //
  // so this should fail validation since
  //   CN is ignored when alt_names are present
  //   SNI of server.everscale.com != *.server.everscale.com
  //
  ClientTLSSocket client("server.everscale.com", _server.secureAddress(), "test", _clientContexts.defaultContext(),
                         true);

  Error error = client.connect();
  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_TRUE(client.connected());
  ASSERT_TRUE(client.secure());
  ASSERT_TRUE(client.isBlocking());

  SSize result = client.send(_message, sizeof(_message));
  ASSERT_GE(result, -1);
  ASSERT_EQ(ESB_TLS_HANDSHAKE_ERROR, LastError());
}

TEST_F(TLSSocketTest, ServerSNI) {
  {
    // Default cert has
    //
    // [alt_names]
    // DNS.1 = *.server.everscale.com
    //
    // so this should fail validation
    //
    ClientTLSSocket client("foo.everscale.com", _server.secureAddress(), "test", _clientContexts.defaultContext(),
                           true);

    Error error = client.connect();
    ASSERT_EQ(ESB_SUCCESS, error);
    ASSERT_TRUE(client.connected());
    ASSERT_TRUE(client.secure());
    ASSERT_TRUE(client.isBlocking());

    SSize result = client.send(_message, sizeof(_message));
    ASSERT_GE(result, -1);
    ASSERT_EQ(ESB_TLS_HANDSHAKE_ERROR, LastError());
  }

  // san1 has:
  //
  // [alt_names]
  // DNS.1 = f*.everscale.com
  // DNS.2 = *z.everscale.com
  // DNS.3 = b*r.everscale.com
  // IP.1 = 1.2.3.4
  // IP.2 = 5.6.7.8
  //
  // Once it's loaded the foo.everscale.com should match f*.everscale.com

  TLSContext::Params params;
  ASSERT_EQ(ESB_SUCCESS,
            _server.contextIndex().indexContext(
                params.privateKeyPath("san1.key").certificatePath("san1.crt").verifyPeerCertificate(false)));

  ClientTLSSocket client("foo.everscale.com", _server.secureAddress(), "test", _clientContexts.defaultContext(), true);

  Error error = client.connect();
  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_TRUE(client.connected());
  ASSERT_TRUE(client.secure());
  ASSERT_TRUE(client.isBlocking());

  SSize result = client.send(_message, sizeof(_message));
  ASSERT_EQ(result, sizeof(_message));

  char buffer[sizeof(_message)];
  result = client.receive(buffer, sizeof(buffer));
  ASSERT_EQ(result, sizeof(buffer));

  ASSERT_TRUE(0 == strcmp(_message, buffer));
}

TEST_F(TLSSocketTest, MutualTLSHappyPath) {
  // san1 has:
  //
  // [alt_names]
  // DNS.1 = f*.everscale.com
  // DNS.2 = *z.everscale.com
  // DNS.3 = b*r.everscale.com
  // IP.1 = 1.2.3.4
  // IP.2 = 5.6.7.8
  //
  // Once it's loaded the foo.everscale.com should match f*.everscale.com

  TLSContext::Params params;
  ASSERT_EQ(ESB_SUCCESS, _server.contextIndex().indexContext(params.privateKeyPath("san1.key")
                                                                 .certificatePath("san1.crt")
                                                                 .caCertificatePath("ca.crt")
                                                                 .verifyPeerCertificate(true)));

  ClientTLSSocket client("foo.everscale.com", _server.secureAddress(), "test", _clientMutualContext, true);

  Error error = client.connect();
  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_TRUE(client.connected());
  ASSERT_TRUE(client.secure());
  ASSERT_TRUE(client.isBlocking());

  SSize result = client.send(_message, sizeof(_message));
  ASSERT_EQ(result, sizeof(_message));

  char buffer[sizeof(_message)];
  result = client.receive(buffer, sizeof(buffer));
  ASSERT_EQ(result, sizeof(buffer));

  ASSERT_TRUE(0 == strcmp(_message, buffer));
}

TEST_F(TLSSocketTest, MutualTLSNoClientCert) {
  // san1 has:
  //
  // [alt_names]
  // DNS.1 = f*.everscale.com
  // DNS.2 = *z.everscale.com
  // DNS.3 = b*r.everscale.com
  // IP.1 = 1.2.3.4
  // IP.2 = 5.6.7.8
  //
  // Once it's loaded the foo.everscale.com should match f*.everscale.com

  TLSContext::Params params;
  ASSERT_EQ(ESB_SUCCESS, _server.contextIndex().indexContext(params.privateKeyPath("san1.key")
                                                                 .certificatePath("san1.crt")
                                                                 .caCertificatePath("ca.crt")
                                                                 .verifyPeerCertificate(true)));

  ClientTLSSocket client("foo.everscale.com", _server.secureAddress(), "test", _clientContexts.defaultContext(), true);

  Error error = client.connect();
  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_TRUE(client.connected());
  ASSERT_TRUE(client.secure());
  ASSERT_TRUE(client.isBlocking());

  SSize result = client.send(_message, sizeof(_message));
  ASSERT_GE(result, -1);
  ASSERT_EQ(ESB_TLS_HANDSHAKE_ERROR, LastError());
}

TEST_F(TLSSocketTest, MutualTLSNoServerCA) {
  // san1 has:
  //
  // [alt_names]
  // DNS.1 = f*.everscale.com
  // DNS.2 = *z.everscale.com
  // DNS.3 = b*r.everscale.com
  // IP.1 = 1.2.3.4
  // IP.2 = 5.6.7.8
  //
  // Once it's loaded the foo.everscale.com should match f*.everscale.com

  TLSContext::Params params;
  ASSERT_EQ(ESB_SUCCESS,
            _server.contextIndex().indexContext(
                params.privateKeyPath("san1.key").certificatePath("san1.crt").verifyPeerCertificate(true)));

  ClientTLSSocket client("foo.everscale.com", _server.secureAddress(), "test", _clientMutualContext, true);

  Error error = client.connect();
  ASSERT_EQ(ESB_SUCCESS, error);
  ASSERT_TRUE(client.connected());
  ASSERT_TRUE(client.secure());
  ASSERT_TRUE(client.isBlocking());

  SSize result = client.send(_message, sizeof(_message));
  ASSERT_GE(result, -1);
  ASSERT_EQ(ESB_TLS_HANDSHAKE_ERROR, LastError());
}
