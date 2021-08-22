#ifndef ES_CONFIG_UTIL_H
#define ES_CONFIG_UTIL_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

#ifndef ESB_AST_TREE_H
#include <ASTTree.h>
#endif

#ifndef ESB_UNIQUE_ID_H
#include <ESBUniqueId.h>
#endif

namespace ES {

class ConfigUtil {
 public:
  inline static ESB::Error FindString(const ESB::AST::Map &map, const char *key, const ESB::AST::String **str) {
    return FindValue(map, key, (const ESB::AST::Element **)str, ESB::AST::Element::STRING);
  }

  inline static ESB::Error FindInteger(const ESB::AST::Map &map, const char *key, const ESB::AST::Integer **integer) {
    return FindValue(map, key, (const ESB::AST::Element **)integer, ESB::AST::Element::INTEGER);
  }

  inline static ESB::Error FindDecimal(const ESB::AST::Map &map, const char *key, const ESB::AST::Decimal **decimal) {
    return FindValue(map, key, (const ESB::AST::Element **)decimal, ESB::AST::Element::DECIMAL);
  }

  inline static ESB::Error FindBoolean(const ESB::AST::Map &map, const char *key, const ESB::AST::Boolean **boolean) {
    return FindValue(map, key, (const ESB::AST::Element **)boolean, ESB::AST::Element::BOOLEAN);
  }

  inline static ESB::Error FindMap(const ESB::AST::Map &map, const char *key, const ESB::AST::Map **map2) {
    return FindValue(map, key, (const ESB::AST::Element **)map2, ESB::AST::Element::MAP);
  }

  inline static ESB::Error FindList(const ESB::AST::Map &map, const char *key, const ESB::AST::List **list) {
    return FindValue(map, key, (const ESB::AST::Element **)list, ESB::AST::Element::LIST);
  }

  static ESB::Error FindUniqueId(const ESB::AST::Map &map, const char *key, ESB::UniqueId &uuid);

 private:
  static ESB::Error FindValue(const ESB::AST::Map &map, const char *key, const ESB::AST::Element **scalar,
                              ESB::AST::Element::Type type);
};

}  // namespace ES

#endif
