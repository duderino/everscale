#ifndef ES_ACTION_H
#define ES_ACTION_H

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

#ifndef ESB_AST_MAP_H
#include <ASTMap.h>
#endif

namespace ES {

class Action : public ESB::EmbeddedListElement {
 public:
  enum Type {
    UNKNOWN,
    MODIFY_REQUEST,
    MODIFY_RESPONSE,
    SEND_REQUEST,
    SEND_RESPONSE,
    CLOSE_CONNECTION,
    RELEASE_SLOT,
    MEASURE,
    LOG,
    MARK
  };

  static ESB::Error Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Action **action);

  virtual ~Action();

  virtual ESB::CleanupHandler *cleanupHandler();

  virtual Type type() const = 0;

 protected:
  // Use Build()
  Action(ESB::Allocator &allocator);

 private:
  ESB::Allocator &_allocator;

  ESB_DEFAULT_FUNCS(Action);
};

}  // namespace ES

#endif
