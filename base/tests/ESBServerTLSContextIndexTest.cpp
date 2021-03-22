#ifndef ESB_SERVER_TLS_CONTEXT_INDEX_H
#include <ESBServerTLSContextIndex.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

class KeyPair {
 public:
  KeyPair(const char *path) {
    snprintf(_key, sizeof(_key), "%s.key", path);
    snprintf(_cert, sizeof(_cert), "%s.crt", path);
  }

  char _key[128];
  char _cert[128];
};

// these are created by base/tests/create-certs.sh.  More alt_names, etc can be added there.

//[alt_names]
// DNS.1 = *.server.everscale.com
static KeyPair Server("server");

//[alt_names]
// DNS.1 = *.client.everscale.com
static KeyPair Client("client");

// no alt_names
static KeyPair Foo("foo");

// no alt_names
static KeyPair Bar("bar");

// no alt_names
static KeyPair Baz("baz");

//[alt_names]
// DNS.1 = f*.everscale.com
// DNS.2 = *z.everscale.com
// DNS.3 = b*r.everscale.com
// IP.1 = 1.2.3.4
// IP.2 = 5.6.7.8
static KeyPair San1("san1");

//[alt_names]
// DNS.1 = q*ux.everscale.com
// DNS.2 = c*.everscale.com
// DNS.3 = *ly.everscale.com
static KeyPair San2("san2");

//[alt_names]
// DNS.1 = f*.server.everscale.com
// DNS.2 = *z.server.everscale.com
// DNS.3 = b*r.server.everscale.com
// IP.1 = 1.2.3.4
// IP.2 = 5.6.7.8
static KeyPair San3("san3");

TEST(ServerTLSContextTest, CommonName) {
  ServerTLSContextPointer context;
  EXPECT_EQ(ESB_SUCCESS,
            ServerTLSContext::Create(context, Foo._key, Foo._cert,
                                     (ServerTLSContext *)SystemAllocator::Instance().allocate(sizeof(ServerTLSContext)),
                                     &SystemAllocator::Instance().cleanupHandler()));
  EXPECT_FALSE(context.isNull());

  char commonName[ESB_MAX_HOSTNAME];
  EXPECT_EQ(ESB_SUCCESS, context->commonName(commonName, sizeof(commonName)));
  EXPECT_EQ(0, strcmp(commonName, "foo.everscale.com"));

  EXPECT_EQ(ESB_SUCCESS,
            ServerTLSContext::Create(context, Bar._key, Bar._cert,
                                     (ServerTLSContext *)SystemAllocator::Instance().allocate(sizeof(ServerTLSContext)),
                                     &SystemAllocator::Instance().cleanupHandler()));
  EXPECT_FALSE(context.isNull());

  EXPECT_EQ(ESB_SUCCESS, context->commonName(commonName, sizeof(commonName)));
  EXPECT_EQ(0, strcmp(commonName, "bar.everscale.com"));
}

TEST(ServerTLSContextTest, SubjectAltNames) {
  ServerTLSContextPointer context;
  EXPECT_EQ(ESB_SUCCESS,
            ServerTLSContext::Create(context, San1._key, San1._cert,
                                     (ServerTLSContext *)SystemAllocator::Instance().allocate(sizeof(ServerTLSContext)),
                                     &SystemAllocator::Instance().cleanupHandler()));
  EXPECT_FALSE(context.isNull());

  EXPECT_EQ(3, context->numSubjectAltNames());

  char subjectAltName[ESB_MAX_HOSTNAME];
  UInt32 position = 0;
  EXPECT_EQ(ESB_SUCCESS, context->subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
  EXPECT_EQ(0, strcmp(subjectAltName, "f*.everscale.com"));
  EXPECT_EQ(ESB_SUCCESS, context->subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
  EXPECT_EQ(0, strcmp(subjectAltName, "*z.everscale.com"));
  EXPECT_EQ(ESB_SUCCESS, context->subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
  EXPECT_EQ(0, strcmp(subjectAltName, "b*r.everscale.com"));
  EXPECT_EQ(ESB_CANNOT_FIND, context->subjectAltName(subjectAltName, sizeof(subjectAltName), &position));

  EXPECT_EQ(ESB_SUCCESS,
            ServerTLSContext::Create(context, San2._key, San2._cert,
                                     (ServerTLSContext *)SystemAllocator::Instance().allocate(sizeof(ServerTLSContext)),
                                     &SystemAllocator::Instance().cleanupHandler()));
  EXPECT_FALSE(context.isNull());

  EXPECT_EQ(3, context->numSubjectAltNames());

  position = 0;
  EXPECT_EQ(ESB_SUCCESS, context->subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
  EXPECT_EQ(0, strcmp(subjectAltName, "q*ux.everscale.com"));
  EXPECT_EQ(ESB_SUCCESS, context->subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
  EXPECT_EQ(0, strcmp(subjectAltName, "c*.everscale.com"));
  EXPECT_EQ(ESB_SUCCESS, context->subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
  EXPECT_EQ(0, strcmp(subjectAltName, "*ly.everscale.com"));
  EXPECT_EQ(ESB_CANNOT_FIND, context->subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
}

TEST(ServerTLSContextTest, NoSubjectAltNames) {
  ServerTLSContextPointer context;
  EXPECT_EQ(ESB_SUCCESS,
            ServerTLSContext::Create(context, Foo._key, Foo._cert,
                                     (ServerTLSContext *)SystemAllocator::Instance().allocate(sizeof(ServerTLSContext)),
                                     &SystemAllocator::Instance().cleanupHandler()));
  EXPECT_FALSE(context.isNull());

  EXPECT_EQ(0, context->numSubjectAltNames());

  char subjectAltName[ESB_MAX_HOSTNAME];
  UInt32 position = 0;
  EXPECT_EQ(ESB_CANNOT_FIND, context->subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
}

class ServerTLSContextIndexTest : public testing::Test {
 public:
  virtual void TearDown() { _index.clear(); }

  ServerTLSContextIndexTest() : _index(42, 3, SystemAllocator::Instance()) {}

 protected:
  ServerTLSContextIndex _index;
};

TEST_F(ServerTLSContextIndexTest, DefaultContext) {
  ServerTLSContextPointer context = _index.defaultContext();
  EXPECT_TRUE(context.isNull());
  EXPECT_EQ(ESB_SUCCESS, _index.indexDefaultContext(Server._key, Server._cert));
  context = _index.defaultContext();
  EXPECT_FALSE(context.isNull());

  char commonName[ESB_MAX_HOSTNAME];
  context->commonName(commonName, sizeof(commonName));
  EXPECT_EQ(0, strcmp(commonName, "server.everscale.com"));

  // CN is server.everscale.com, but the SAN of *.server.everscale.com doesn't match
  EXPECT_EQ(ESB_CANNOT_FIND, _index.matchContext("server.everscale.com", context));

  EXPECT_EQ(ESB_SUCCESS, _index.matchContext("foo.server.everscale.com", context));
  context->commonName(commonName, sizeof(commonName));
  EXPECT_EQ(0, strcmp(commonName, "server.everscale.com"));

  EXPECT_EQ(ESB_SUCCESS, _index.matchContext("bar.server.everscale.com", context));
  context->commonName(commonName, sizeof(commonName));
  EXPECT_EQ(0, strcmp(commonName, "server.everscale.com"));
}

TEST_F(ServerTLSContextIndexTest, MostSpecificMatch) {
  ServerTLSContextPointer context;
  char commonName[ESB_MAX_HOSTNAME];

  // CN=server.everscale.com
  // [alt_names]
  // DNS.1 = *.server.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.indexDefaultContext(Server._key, Server._cert));

  // CN=foo.everscale.com
  // no alt_names,
  ASSERT_EQ(ESB_SUCCESS, _index.indexContext(Foo._key, Foo._cert));

  // CN=san1.everscale.com
  // [alt_names]
  // DNS.1 = f*.everscale.com
  // DNS.2 = *z.everscale.com
  // DNS.3 = b*r.everscale.com
  // IP.1 = 1.2.3.4
  // IP.2 = 5.6.7.8
  ASSERT_EQ(ESB_SUCCESS, _index.indexContext(San1._key, San1._cert));

  // CN=san2.everscale.com
  // [alt_names]
  // DNS.1 = q*ux.everscale.com
  // DNS.2 = c*.everscale.com
  // DNS.3 = *ly.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.indexContext(San2._key, San2._cert));

  // CN=san3.everscale.com
  // [alt_names]
  // DNS.1 = f*.server.everscale.com
  // DNS.2 = *z.server.everscale.com
  // DNS.3 = b*r.server.everscale.com
  // IP.1 = 1.2.3.4
  // IP.2 = 5.6.7.8
  ASSERT_EQ(ESB_SUCCESS, _index.indexContext(San3._key, San3._cert));

  // CN=server.everscale.com ignored because Server had SAN of *.server.everscale.com
  ASSERT_EQ(ESB_CANNOT_FIND, _index.matchContext("server.everscale.com", context));

  // CN=foo.everscale.com more specific than f*.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("foo.everscale.com", context));
  context->commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "foo.everscale.com"));

  // But f*.everscale.com is the best match for fooo.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("fooo.everscale.com", context));
  context->commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "san1.everscale.com"));

  // And b*r.everscale.com is the best match for bar.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("bar.everscale.com", context));
  context->commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "san1.everscale.com"));

  // Until we insert an exact match for bar.everscale.com

  // CN=foo.everscale.com
  // no alt_names,
  ASSERT_EQ(ESB_SUCCESS, _index.indexContext(Bar._key, Bar._cert));
  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("bar.everscale.com", context));
  context->commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "bar.everscale.com"));

  //
  // And additional checks
  //

  // And q*ux.everscale.com is the best match for qux.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("qux.everscale.com", context));
  context->commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "san2.everscale.com"));

  // And q*ux.everscale.com is the best match for quux.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("quux.everscale.com", context));
  context->commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "san2.everscale.com"));

  // And f*.server.everscale.com is the best match for foo.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("foo.server.everscale.com", context));
  context->commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "san3.everscale.com"));
}
