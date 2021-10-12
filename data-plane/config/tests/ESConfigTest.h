#ifndef ES_ENTITY_TEST_H
#define ES_ENTITY_TEST_H

#ifndef ESB_AST_TREE_H
#include <ASTTree.h>
#endif

#include <gtest/gtest.h>

namespace ES {

class ConfigTest : public ::testing::Test {
 public:
  ConfigTest(ESB::Allocator &allocator = ESB::SystemAllocator::Instance());
  virtual ~ConfigTest();

 protected:
  ESB::Error parseFile(const char *path, ESB::AST::Tree &tree);

  ESB::Error parseString(const char *str, ESB::AST::Tree &tree);

  ESB::Allocator &_allocator;

  ESB_DISABLE_AUTO_COPY(ConfigTest);
};

}  // namespace ES

#endif