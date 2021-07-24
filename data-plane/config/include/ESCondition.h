#ifndef ES_CONDITION_H
#define ES_CONDITION_H

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

#ifndef ESB_AST_MAP_H
#include <ASTMap.h>
#endif

namespace ES {

class Condition : public ESB::EmbeddedListElement {
 public:
  enum Type { UNKNOWN, ACQUIRE_SLOT, PROPERTY_MATCH, RESULT_MATCH, MARK_MARK, WAF_MATCH };

  static ESB::Error Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Condition **condition);

  virtual ~Condition();

  virtual ESB::CleanupHandler *cleanupHandler();

  virtual Type type() const = 0;

 protected:
  // Use Build()
  Condition(ESB::Allocator &allocator);

 private:
  ESB::Allocator &_allocator;

  ESB_DEFAULT_FUNCS(Condition);
};

}  // namespace ES

#endif
