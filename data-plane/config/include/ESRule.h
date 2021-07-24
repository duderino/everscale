#ifndef ES_RULE_H
#define ES_RULE_H

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

namespace ES {

class Rule : public ESB::EmbeddedListElement {
 public:
  Rule(ESB::Allocator &allocator);

  virtual ~Rule();

  virtual ESB::CleanupHandler *cleanupHandler();

  inline ESB::EmbeddedList &actions() { return _actions; }

  inline const ESB::EmbeddedList &actions() const { return _actions; }

  inline ESB::EmbeddedList &conditions() { return _conditions; }

  inline const ESB::EmbeddedList &conditions() const { return _conditions; }

 private:
  ESB::Allocator &_allocator;
  ESB::EmbeddedList _conditions;
  ESB::EmbeddedList _actions;

  ESB_DEFAULT_FUNCS(Rule);
};

}  // namespace ES

#endif
