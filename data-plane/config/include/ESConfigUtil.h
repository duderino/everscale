#ifndef ES_CONFIG_UTIL_H
#define ES_CONFIG_UTIL_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

#ifndef ESB_AST_TREE_H
#include <ASTTree.h>
#endif

namespace ES {

class ConfigUtil {
 public:
  static ESB::Error StringValue(const ESB::AST::Map &map, const char *key, const ESB::AST::String **str);
};

}  // namespace ES

#endif
