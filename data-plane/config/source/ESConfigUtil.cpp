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

ESB::Error ConfigUtil::FindUniqueId(const ESB::AST::Map &map, const char *key, ESB::UniqueId &uuid) {
  const ESB::AST::String *str = NULL;
  ESB::Error error = ConfigUtil::FindString(map, key, &str);
  if (ESB_SUCCESS != error) {
    return error;
  }
  assert(str);

  ESB::UInt128 id = 0;
  error = ESB::UniqueId::Parse(str->value(), &id);
  if (ESB_SUCCESS != error) {
    return error;
  }

  uuid.set(id);
  return ESB_SUCCESS;
}

ESB::Error ConfigUtil::FindValue(const ESB::AST::Map &map, const char *key, const ESB::AST::Element **scalar,
                                 ESB::AST::Element::Type type) {
  if (!key || !scalar) {
    return ESB_NULL_POINTER;
  }

  const ESB::AST::Element *e = map.find(key);

  if (!e) {
    return ESB_MISSING_FIELD;
  }

  if (type != e->type()) {
    return ESB_INVALID_FIELD;
  }

  *scalar = e;
  return ESB_SUCCESS;
}

}  // namespace ES
