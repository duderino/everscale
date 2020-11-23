#include <gtest/gtest.h>
#include <openssl/ssl.h>

TEST(BoringSSL, SanityCheck) { EXPECT_EQ(1, OPENSSL_init_ssl(OPENSSL_INIT_SSL_DEFAULT, NULL)); }
