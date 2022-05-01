#ifndef ES_ENTITY_H
#include <ESEntity.h>
#endif

#ifndef ES_CONFIG_TEST_H
#include "ESConfigTest.h"
#endif

#include <gtest/gtest.h>

using namespace ES;

#define UUID1 "ec23c29b-605e-4b0b-8bae-a4c4692e6164"
#define UUID2 "518aa91c-1b06-4364-a0bd-850a04563fa9"
#define UUID3 "cdad51ab-9f54-4b9a-bbb5-38568f978019"
#define UUID4 "81607fb3-7453-4372-995a-5e1f1317fd23"
#define UUID5 "0d50e7dd-2e76-4d3e-b401-2826265893a8"
#define UUID6 "e46160d1-6489-4ee9-b1c6-98ac27a00d30"

class EntityTest : public ConfigTest {
 public:
  EntityTest() {}
  virtual ~EntityTest() {}

  virtual void SetUp() {
    ASSERT_EQ(ESB_SUCCESS, ESB::UniqueId::Parse(UUID1, _uuid1));
    ASSERT_EQ(ESB_SUCCESS, ESB::UniqueId::Parse(UUID2, _uuid2));
    ASSERT_EQ(ESB_SUCCESS, ESB::UniqueId::Parse(UUID3, _uuid3));
    ASSERT_EQ(ESB_SUCCESS, ESB::UniqueId::Parse(UUID4, _uuid4));
    ASSERT_EQ(ESB_SUCCESS, ESB::UniqueId::Parse(UUID5, _uuid5));
    ASSERT_EQ(ESB_SUCCESS, ESB::UniqueId::Parse(UUID6, _uuid6));
  }

 protected:
  ESB::UniqueId _uuid1;
  ESB::UniqueId _uuid2;
  ESB::UniqueId _uuid3;
  ESB::UniqueId _uuid4;
  ESB::UniqueId _uuid5;
  ESB::UniqueId _uuid6;

  ESB_DISABLE_AUTO_COPY(EntityTest);
};

TEST_F(EntityTest, ParseTLSContext) {
  const char *conf =
      "            {"
      "              \"id\": \"" UUID1
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
  ASSERT_EQ(_uuid1, entity->id());
  ASSERT_EQ(Entity::TLS_CTX, entity->type());

  TLSContextEntity *tlsContext = (TLSContextEntity *)entity;
  ASSERT_TRUE(0 == strcmp(tlsContext->keyPath(), "/foo.key"));
  ASSERT_TRUE(0 == strcmp(tlsContext->certPath(), "/bar.crt"));
  ASSERT_TRUE(0 == strcmp(tlsContext->caPath(), "/baz.crt"));
  ASSERT_EQ(tlsContext->peerVerification(), ESB::TLSContext::VERIFY_ALWAYS);
  ASSERT_EQ(tlsContext->certificateChainDepth(), 42);

  entity->cleanupHandler()->destroy(entity);
}

TEST_F(EntityTest, ParseTLSContextIndex) {
  const char *conf =
      "            {"
      "              \"id\": \"" UUID1
      "\","
      "              \"type\": \"TLS_IDX\","
      "              \"default_context\": \"" UUID2
      "\","
      "              \"contexts\": ["
      "                  \"" UUID3
      "\","
      "                  \"" UUID4
      "\","
      "                  \"" UUID5
      "\","
      "                  \"" UUID6
      "\""
      "              ]"
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
  ASSERT_EQ(_uuid1, entity->id());
  ASSERT_EQ(Entity::TLS_IDX, entity->type());

  TLSContextIndexEntity *tlsContextIdx = (TLSContextIndexEntity *)entity;
  ASSERT_EQ(tlsContextIdx->defaultContext(), _uuid2);
  ASSERT_EQ(tlsContextIdx->numContexts(), 4);
  ASSERT_EQ(tlsContextIdx->contexts()[0], _uuid3);
  ASSERT_EQ(tlsContextIdx->contexts()[1], _uuid4);
  ASSERT_EQ(tlsContextIdx->contexts()[2], _uuid5);
  ASSERT_EQ(tlsContextIdx->contexts()[3], _uuid6);

  entity->cleanupHandler()->destroy(entity);
}

TEST_F(EntityTest, ParseTLSContextIndex2) {
  const char *conf =
      "            {"
      "              \"id\": \"" UUID1
      "\","
      "              \"type\": \"TLS_IDX\","
      "              \"default_context\": \"" UUID2
      "\","
      "              \"contexts\": []"
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
  ASSERT_EQ(_uuid1, entity->id());
  ASSERT_EQ(Entity::TLS_IDX, entity->type());

  TLSContextIndexEntity *tlsContextIdx = (TLSContextIndexEntity *)entity;
  ASSERT_EQ(tlsContextIdx->defaultContext(), _uuid2);
  ASSERT_EQ(tlsContextIdx->numContexts(), 0);
  ASSERT_FALSE(tlsContextIdx->contexts());

  entity->cleanupHandler()->destroy(entity);
}

TEST_F(EntityTest, ParseTLSContextIndex3) {
  const char *conf =
      "            {"
      "              \"id\": \"" UUID1
      "\","
      "              \"type\": \"TLS_IDX\","
      "              \"default_context\": \"" UUID2
      "\""
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
  ASSERT_EQ(_uuid1, entity->id());
  ASSERT_EQ(Entity::TLS_IDX, entity->type());

  TLSContextIndexEntity *tlsContextIdx = (TLSContextIndexEntity *)entity;
  ASSERT_EQ(tlsContextIdx->defaultContext(), _uuid2);
  ASSERT_EQ(tlsContextIdx->numContexts(), 0);
  ASSERT_FALSE(tlsContextIdx->contexts());

  entity->cleanupHandler()->destroy(entity);
}