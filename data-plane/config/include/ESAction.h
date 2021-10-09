#ifndef ES_ACTION_H
#define ES_ACTION_H

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

#ifndef ESB_AST_MAP_H
#include <ASTMap.h>
#endif

#ifndef ESB_UNIQUE_ID_H
#include <ESBUniqueId.h>
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
    MARK,
    TRANSITION
  };

  static ESB::Error Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Action **action);

  virtual ~Action();

  virtual ESB::CleanupHandler *cleanupHandler();

  virtual Type type() const = 0;

 protected:
  // Use Build()
  Action(ESB::Allocator &allocator);

  ESB::Allocator &_allocator;

  ESB_DISABLE_AUTO_COPY(Action);
};

class ModifyRequestAction : public Action {
 public:
  enum ActionType { CREATE = 0, UPDATE = 1, DELETE = 2, SANITIZE = 3, CANONICALIZE = 4 };

  enum TargetType { PROTOCOL = 0, HEADER = 1, PATH = 2, ATTRIBUTE = 3 };

  static ESB::Error Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Action **action);

  virtual ~ModifyRequestAction();

  virtual Type type() const;

 private:
  // Use Build()
  ModifyRequestAction(ESB::Allocator &allocator);

  ESB_DEFAULT_FUNCS(ModifyRequestAction);
};

class SendResponseAction : public Action {
 public:
  static ESB::Error Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Action **action);

  virtual ~SendResponseAction();

  virtual Type type() const;

  /**
   * Get the mandatory status code
   *
   * @return The status code
   */
  inline ESB::UInt16 statusCode() const { return _statusCode; }

  /**
   * Get the optional reason phrase
   *
   * @return Return the reason phrase if set, NULL otherwise
   */
  inline const char *reasonPhrase() const { return _reasonPhrase; }

 private:
  // Use Build()
  SendResponseAction(ESB::Allocator &allocator, ESB::UInt16 statusCode, char *reasonPhrase = NULL);

  ESB::UInt16 _statusCode;
  char *_reasonPhrase;

  ESB_DEFAULT_FUNCS(SendResponseAction);
};

class TransitionAction : public Action {
 public:
  static ESB::Error Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Action **action);

  virtual ~TransitionAction();

  virtual Type type() const;

  inline const ESB::UniqueId &destination() const { return _destination; }

 private:
  // Use Build()
  TransitionAction(ESB::Allocator &allocator, const ESB::UniqueId &destination);

  ESB::UniqueId _destination;

  ESB_DEFAULT_FUNCS(TransitionAction);
};

}  // namespace ES

#endif
