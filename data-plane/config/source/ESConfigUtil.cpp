#ifndef ES_CONFIG_UTIL_H
#include <ESConfigUtil.h>
#endif

#ifndef ESB_BUFFERED_FILE_H
#include <ESBBufferedFile.h>
#endif

#ifndef ESB_JSON_PARSER_H
#include <ESBJsonParser.h>
#endif

#ifndef ESB_AST_TREE_H
#include <ASTTree.h>
#endif

namespace ES {

ESB::Error ConfigUtil::StringValue(const ESB::AST::Map &map, const char *key, const ESB::AST::String **str) {
  if (!key || !str) {
    return ESB_NULL_POINTER;
  }

  const ESB::AST::Element *e = map.find(key);

  if (!e) {
    return ESB_MISSING_FIELD;
  }

  if (ESB::AST::Element::STRING != e->type()) {
    return ESB_INVALID_FIELD;
  }

  *str = (const ESB::AST::String *)e;
  return ESB_SUCCESS;
}

}  // namespace ES
