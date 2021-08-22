#ifndef ES_ACTION_H
#include <ESAction.h>
#endif

#ifndef ES_ENTITY_TEST_H
#include "ESEntityTest.h"
#endif

#include <gtest/gtest.h>

#define UUID "0f42a1d1-cb6e-4a7b-be44-35fe34c6d5e1"

using namespace ES;

class ActionTest : public EntityTest {
 public:
  ActionTest() { assert(ESB_SUCCESS == ESB::UniqueId::Parse(UUID, _uuid)); }
  virtual ~ActionTest() {}

 protected:
  ESB::UniqueId _uuid;

  ESB_DISABLE_AUTO_COPY(ActionTest);
};

TEST_F(ActionTest, ParseTransition) {
  const char *conf =
      "            {"
      "              \"type\": \"TRANSITION\","
      "              \"destination\": \"" UUID
      "\""
      "            }";
  ESB::AST::Tree tree;
  ASSERT_EQ(ESB_SUCCESS, parseString(conf, tree));
  ASSERT_TRUE(tree.root());
  ASSERT_EQ(tree.root()->type(), ESB::AST::Element::MAP);
  ESB::AST::Map &map = *(ESB::AST::Map *)tree.root();

  Action *action = NULL;
  ASSERT_EQ(ESB_SUCCESS, Action::Build(map, ESB::SystemAllocator::Instance(), &action));
  ASSERT_TRUE(action);
  ASSERT_TRUE(action->cleanupHandler());
  ASSERT_EQ(Action::TRANSITION, action->type());

  TransitionAction *transition = (TransitionAction *)action;
  ASSERT_EQ(_uuid, transition->destination());

  action->cleanupHandler()->destroy(action);
}

TEST_F(ActionTest, ParseSendResponse) {
  const char *conf =
      "            {"
      "              \"type\": \"SEND_RESPONSE\","
      "              \"status_code\": 404,"
      "              \"reason_phrase\": \"Not Found\""
      "            }";
  ESB::AST::Tree tree;
  ASSERT_EQ(ESB_SUCCESS, parseString(conf, tree));
  ASSERT_TRUE(tree.root());
  ASSERT_EQ(tree.root()->type(), ESB::AST::Element::MAP);
  ESB::AST::Map &map = *(ESB::AST::Map *)tree.root();

  Action *action = NULL;
  ASSERT_EQ(ESB_SUCCESS, Action::Build(map, ESB::SystemAllocator::Instance(), &action));
  ASSERT_TRUE(action);
  ASSERT_TRUE(action->cleanupHandler());
  ASSERT_EQ(Action::SEND_RESPONSE, action->type());

  SendResponseAction *sendResponse = (SendResponseAction *)action;
  ASSERT_EQ(404, sendResponse->statusCode());
  const char *reasonPhrase = sendResponse->reasonPhrase();
  ASSERT_EQ(0, strcmp("Not Found", reasonPhrase));

  action->cleanupHandler()->destroy(action);
}