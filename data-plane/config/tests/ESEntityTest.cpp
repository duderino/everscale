#ifndef ES_ENTITY_H
#include <ESEntity.h>
#endif

#ifndef ES_CONFIG_TEST_H
#include "ESConfigTest.h"
#endif

#include <gtest/gtest.h>

#define UUID "ec23c29b-605e-4b0b-8bae-a4c4692e6164"

using namespace ES;

class EntityTest : public ConfigTest {
 public:
  EntityTest() {}
  virtual ~EntityTest() {}

  virtual void SetUp() { ASSERT_EQ(ESB_SUCCESS, ESB::UniqueId::Parse(UUID, _uuid)); }

 protected:
  ESB::UniqueId _uuid;

  ESB_DISABLE_AUTO_COPY(EntityTest);
};

TEST_F(EntityTest, ParseTLSContext) {
  const char *conf =
      "            {"
      "              \"id\": \"" UUID
      "\","
      "              \"type\": \"TLS_CTX\","
      "              \"key_path\": \"/foo.key\","
      "              \"cert_path\": \"/bar.crt\","
      "              \"ca_path\": \"/baz.crt\","
      "              \"peer_verification\": \"VERIFY_ALWAYS\","
      "              \"certificate_chain_depth\": 42"
      "            }";
  ESB::AST::Tree tree;
  ASSERT_EQ(ESB_SUCCESS, parseString(conf, tree));
  ASSERT_TRUE(tree.root());
  ASSERT_EQ(tree.root()->type(), ESB::AST::Element::MAP);
  ESB::AST::Map &map = *(ESB::AST::Map *)tree.root();

  Entity *entity = NULL;
  ASSERT_EQ(ESB_SUCCESS, Entity::Build(map, ESB::SystemAllocator::Instance(), &entity));
  ASSERT_TRUE(entity);
  ASSERT_TRUE(entity->cleanupHandler());
  ASSERT_EQ(_uuid, entity->id());
  ASSERT_EQ(Entity::TLS_CTX, entity->type());

  TLSContextEntity *tlsContext = (TLSContextEntity *)entity;
  ASSERT_TRUE(0 == strcmp(tlsContext->keyPath(), "/foo.key"));
  ASSERT_TRUE(0 == strcmp(tlsContext->certPath(), "/bar.crt"));
  ASSERT_TRUE(0 == strcmp(tlsContext->caPath(), "/baz.crt"));
  ASSERT_EQ(tlsContext->peerVerification(), ESB::TLSContext::VERIFY_ALWAYS);
  ASSERT_EQ(tlsContext->certificateChainDepth(), 42);

  entity->cleanupHandler()->destroy(entity);
}
