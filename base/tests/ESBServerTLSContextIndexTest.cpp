#ifndef ESB_SERVER_TLS_CONTEXT_INDEX_H
#include <ESBServerTLSContextIndex.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

// these are created by base/tests/create-certs.sh.  More alt_names, etc can be added there.
static const char *DEFAULT_SERVER_CERT_PATH = "server.crt";
static const char *DEFAULT_SERVER_KEY_PATH = "server.key";

TEST(ServerTLSContext, CommonName) {
  ServerTLSContextIndex index(42, 3, SystemAllocator::Instance());

  {
    ServerTLSContextPointer context;
    EXPECT_EQ(ESB_SUCCESS, ServerTLSContext::Create(
                               context, DEFAULT_SERVER_KEY_PATH, DEFAULT_SERVER_CERT_PATH,
                               (ServerTLSContext *)SystemAllocator::Instance().allocate(sizeof(ServerTLSContext)),
                               &SystemAllocator::Instance().cleanupHandler()));
    EXPECT_FALSE(context.isNull());

    char commonName[ESB_MAX_HOSTNAME];
    EXPECT_EQ(ESB_SUCCESS, context->commonName(commonName, sizeof(commonName)));
    EXPECT_EQ(0, strcmp(commonName, "server.everscale.com"));
  }
}

TEST(ServerTLSContext, SubjectAltNames) {
  ServerTLSContextIndex index(42, 3, SystemAllocator::Instance());

  {
    ServerTLSContextPointer context;
    EXPECT_EQ(ESB_SUCCESS, ServerTLSContext::Create(
                               context, DEFAULT_SERVER_KEY_PATH, DEFAULT_SERVER_CERT_PATH,
                               (ServerTLSContext *)SystemAllocator::Instance().allocate(sizeof(ServerTLSContext)),
                               &SystemAllocator::Instance().cleanupHandler()));
    EXPECT_FALSE(context.isNull());

    EXPECT_EQ(4, context->numSubjectAltNames());

    char subjectAltName[ESB_MAX_HOSTNAME];
    UInt32 position = 0;
    EXPECT_EQ(ESB_SUCCESS, context->subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
    EXPECT_EQ(0, strcmp(subjectAltName, "*.server.everscale.com"));
    EXPECT_EQ(ESB_SUCCESS, context->subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
    EXPECT_EQ(0, strcmp(subjectAltName, "f*.everscale.com"));
    EXPECT_EQ(ESB_SUCCESS, context->subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
    EXPECT_EQ(0, strcmp(subjectAltName, "*z.everscale.com"));
    EXPECT_EQ(ESB_SUCCESS, context->subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
    EXPECT_EQ(0, strcmp(subjectAltName, "b*r.everscale.com"));
    EXPECT_EQ(ESB_CANNOT_FIND, context->subjectAltName(subjectAltName, sizeof(subjectAltName), &position));
  }
}