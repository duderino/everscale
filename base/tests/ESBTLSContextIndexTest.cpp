#ifndef ESB_SERVER_TLS_CONTEXT_INDEX_H
#include <ESBServerTLSContextIndex.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(ServerTLSContextTest, CommonName) {
  TLSContextPointer context;
  TLSContext::Params params;
  ASSERT_EQ(ESB_SUCCESS, TLSContext::Create(context, params.privateKeyPath("foo.key").certificatePath("foo.crt"),
                                            (TLSContext *)SystemAllocator::Instance().allocate(sizeof(TLSContext)),
                                            &SystemAllocator::Instance().cleanupHandler()));
  ASSERT_FALSE(context.isNull());

  char commonName[ESB_MAX_HOSTNAME];
  ASSERT_EQ(ESB_SUCCESS, context->certificate().commonName(commonName, sizeof(commonName)));
  ASSERT_EQ(0, strcmp(commonName, "foo.everscale.com"));

  ASSERT_EQ(ESB_SUCCESS, TLSContext::Create(context, params.privateKeyPath("bar.key").certificatePath("bar.crt"),
                                            (TLSContext *)SystemAllocator::Instance().allocate(sizeof(TLSContext)),
                                            &SystemAllocator::Instance().cleanupHandler()));
  ASSERT_FALSE(context.isNull());

  ASSERT_EQ(ESB_SUCCESS, context->certificate().commonName(commonName, sizeof(commonName)));
  ASSERT_EQ(0, strcmp(commonName, "bar.everscale.com"));
}

TEST(ServerTLSContextTest, SubjectAltNames) {
  TLSContextPointer context;
  TLSContext::Params params;
  ASSERT_EQ(ESB_SUCCESS, TLSContext::Create(context, params.privateKeyPath("san1.key").certificatePath("san1.crt"),
                                            (TLSContext *)SystemAllocator::Instance().allocate(sizeof(TLSContext)),
                                            &SystemAllocator::Instance().cleanupHandler()));
  ASSERT_FALSE(context.isNull());

  ASSERT_EQ(3, context->certificate().numSubjectAltNames());

  char subjectAltName[ESB_MAX_HOSTNAME];
  UInt32 position = 0;
  ASSERT_EQ(ESB_SUCCESS, context->certificate().subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
  ASSERT_EQ(0, strcmp(subjectAltName, "f*.everscale.com"));
  ASSERT_EQ(ESB_SUCCESS, context->certificate().subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
  ASSERT_EQ(0, strcmp(subjectAltName, "b*r.everscale.com"));
  ASSERT_EQ(ESB_SUCCESS, context->certificate().subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
  ASSERT_EQ(0, strcmp(subjectAltName, "*z.everscale.com"));
  ASSERT_EQ(ESB_CANNOT_FIND, context->certificate().subjectAltName(subjectAltName, sizeof(subjectAltName), &position));

  ASSERT_EQ(ESB_SUCCESS, TLSContext::Create(context, params.privateKeyPath("san2.key").certificatePath("san2.crt"),
                                            (TLSContext *)SystemAllocator::Instance().allocate(sizeof(TLSContext)),
                                            &SystemAllocator::Instance().cleanupHandler()));
  ASSERT_FALSE(context.isNull());

  ASSERT_EQ(3, context->certificate().numSubjectAltNames());

  position = 0;
  ASSERT_EQ(ESB_SUCCESS, context->certificate().subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
  ASSERT_EQ(0, strcmp(subjectAltName, "q*ux.everscale.com"));
  ASSERT_EQ(ESB_SUCCESS, context->certificate().subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
  ASSERT_EQ(0, strcmp(subjectAltName, "c*.everscale.com"));
  ASSERT_EQ(ESB_SUCCESS, context->certificate().subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
  ASSERT_EQ(0, strcmp(subjectAltName, "*ly.everscale.com"));
  ASSERT_EQ(ESB_CANNOT_FIND, context->certificate().subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
}

TEST(ServerTLSContextTest, NoSubjectAltNames) {
  TLSContextPointer context;
  TLSContext::Params params;
  ASSERT_EQ(ESB_SUCCESS, TLSContext::Create(context, params.privateKeyPath("foo.key").certificatePath("foo.crt"),
                                            (TLSContext *)SystemAllocator::Instance().allocate(sizeof(TLSContext)),
                                            &SystemAllocator::Instance().cleanupHandler()));
  ASSERT_FALSE(context.isNull());

  ASSERT_EQ(0, context->certificate().numSubjectAltNames());

  char subjectAltName[ESB_MAX_HOSTNAME];
  UInt32 position = 0;
  ASSERT_EQ(ESB_CANNOT_FIND, context->certificate().subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
}

class ServerTLSContextIndexTest : public testing::Test {
 public:
  virtual void TearDown() { _index.clear(); }

  ServerTLSContextIndexTest() : _index(42, 3, SystemAllocator::Instance()) {}

 protected:
  ServerTLSContextIndex _index;
};

TEST_F(ServerTLSContextIndexTest, DefaultContext) {
  TLSContext::Params params;
  TLSContextPointer context = _index.defaultContext();
  ASSERT_TRUE(context.isNull());
  ASSERT_EQ(ESB_SUCCESS, _index.indexDefaultContext(params.privateKeyPath("server.key").certificatePath("server.crt")));
  context = _index.defaultContext();
  ASSERT_FALSE(context.isNull());

  char commonName[ESB_MAX_HOSTNAME];
  context->certificate().commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "server.everscale.com"));

  // CN is server.everscale.com, but the SAN of *.server.everscale.com doesn't match
  ASSERT_EQ(ESB_CANNOT_FIND, _index.matchContext("server.everscale.com", context));

  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("foo.server.everscale.com", context));
  context->certificate().commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "server.everscale.com"));

  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("bar.server.everscale.com", context));
  context->certificate().commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "server.everscale.com"));
}

TEST_F(ServerTLSContextIndexTest, MostSpecificMatch) {
  TLSContextPointer context;
  TLSContext::Params params;
  char commonName[ESB_MAX_HOSTNAME];

  // CN=server.everscale.com
  // [alt_names]
  // DNS.1 = *.server.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.indexDefaultContext(params.privateKeyPath("server.key").certificatePath("server.crt")));

  // CN=foo.everscale.com
  // no alt_names,
  ASSERT_EQ(ESB_SUCCESS, _index.indexContext(params.privateKeyPath("foo.key").certificatePath("foo.crt")));

  // CN=san1.everscale.com
  // [alt_names]
  // DNS.1 = f*.everscale.com
  // DNS.2 = *z.everscale.com
  // DNS.3 = b*r.everscale.com
  // IP.1 = 1.2.3.4
  // IP.2 = 5.6.7.8
  ASSERT_EQ(ESB_SUCCESS, _index.indexContext(params.privateKeyPath("san1.key").certificatePath("san1.crt")));

  // CN=san2.everscale.com
  // [alt_names]
  // DNS.1 = q*ux.everscale.com
  // DNS.2 = c*.everscale.com
  // DNS.3 = *ly.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.indexContext(params.privateKeyPath("san2.key").certificatePath("san2.crt")));

  // CN=san3.everscale.com
  // [alt_names]
  // DNS.1 = f*.server.everscale.com
  // DNS.2 = *z.server.everscale.com
  // DNS.3 = b*r.server.everscale.com
  // IP.1 = 1.2.3.4
  // IP.2 = 5.6.7.8
  ASSERT_EQ(ESB_SUCCESS, _index.indexContext(params.privateKeyPath("san3.key").certificatePath("san3.crt")));

  // CN=server.everscale.com ignored because Server had SAN of *.server.everscale.com
  ASSERT_EQ(ESB_CANNOT_FIND, _index.matchContext("server.everscale.com", context));

  // CN=foo.everscale.com more specific than f*.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("foo.everscale.com", context));
  context->certificate().commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "foo.everscale.com"));

  // But f*.everscale.com is the best match for fooo.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("fooo.everscale.com", context));
  context->certificate().commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "san1.everscale.com"));

  // And b*r.everscale.com is the best match for bar.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("bar.everscale.com", context));
  context->certificate().commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "san1.everscale.com"));

  // Until we insert an exact match for bar.everscale.com

  // CN=foo.everscale.com
  // no alt_names,
  ASSERT_EQ(ESB_SUCCESS, _index.indexContext(params.privateKeyPath("bar.key").certificatePath("bar.crt")));
  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("bar.everscale.com", context));
  context->certificate().commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "bar.everscale.com"));

  //
  // And additional checks
  //

  // And q*ux.everscale.com is the best match for qux.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("qux.everscale.com", context));
  context->certificate().commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "san2.everscale.com"));

  // And q*ux.everscale.com is the best match for quux.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("quux.everscale.com", context));
  context->certificate().commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "san2.everscale.com"));

  // And f*.server.everscale.com is the best match for foo.everscale.com
  ASSERT_EQ(ESB_SUCCESS, _index.matchContext("foo.server.everscale.com", context));
  context->certificate().commonName(commonName, sizeof(commonName));
  ASSERT_EQ(0, strcmp(commonName, "san3.everscale.com"));
}

TEST(ServerTLSContextTest, KeyCertMismatch) {
  TLSContextPointer context;
  TLSContext::Params params;
  TLSContext *block = (TLSContext *)SystemAllocator::Instance().allocate(sizeof(TLSContext));
  ASSERT_EQ(ESB_GENERAL_TLS_ERROR,
            TLSContext::Create(context, params.privateKeyPath("foo.key").certificatePath("bar.crt"), block,
                               &SystemAllocator::Instance().cleanupHandler()));
  ASSERT_TRUE(context.isNull());
  SystemAllocator::Instance().deallocate(block);
}
