#include <gtest/gtest.h>
#include <openssl/opensslconf.h>
#include <openssl/ssl.h>

#ifndef OPENSSL_THREADS
#error "BoringSSL not configured for multi-threaded use"
#endif

TEST(ConnectedTLSSocket, SanityCheck) { EXPECT_EQ(1, OPENSSL_init_ssl(OPENSSL_INIT_SSL_DEFAULT, NULL)); }
