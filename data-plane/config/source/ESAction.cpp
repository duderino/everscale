#ifndef ES_ACTION_H
#include <ESAction.h>
#endif

#ifndef ES_CONFIG_UTIL_H
#include <ESConfigUtil.h>
#endif

#ifndef ESB_STRING_H
#include <ESBString.h>
#endif

namespace ES {

// Must be in same order as ActionType
static const char *ActionTypeStrings[] = {"",
                                          "MODIFY_REQUEST",
                                          "MODIFY_RESPONSE",
                                          "SEND_REQUEST",
                                          "SEND_RESPONSE",
                                          "CLOSE_CONNECTION",
                                          "RELEASE_SLOT",
                                          "MEASURE",
                                          "LOG",
                                          "MARK",
                                          "TRANSITION"};

static Action::Type ParseActionType(const char *str) {
  if (!str) {
    return Action::UNKNOWN;
  }

  for (int i = 0; i < sizeof(ActionTypeStrings) / sizeof(char *); ++i) {
    if (0 == strcasecmp(str, ActionTypeStrings[i])) {
      return (Action::Type)i;
    }
  }

  return Action::UNKNOWN;
}

Action::Action(ESB::Allocator &allocator) : _allocator(allocator) {}

Action::~Action() {}

ESB::CleanupHandler *Action::cleanupHandler() { return &_allocator.cleanupHandler(); }

ESB::Error Action::Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Action **action) {
  if (!action) {
    return ESB_NULL_POINTER;
  }

  Type type = UNKNOWN;

  {
    const ESB::AST::String *str = NULL;
    ESB::Error error = ConfigUtil::FindString(map, "type", &str);
    if (ESB_SUCCESS != error) {
      return error;
    }

    type = ParseActionType(str->value());
    if (UNKNOWN == type) {
      return ESB_INVALID_FIELD;
    }
  }

  switch (type) {
    case SEND_RESPONSE:
      return SendResponseAction::Build(map, allocator, action);
    case TRANSITION:
      return TransitionAction::Build(map, allocator, action);
    default:
      return ESB_NOT_IMPLEMENTED;
  }
}

TransitionAction::TransitionAction(ESB::Allocator &allocator, const ESB::UniqueId &destination)
    : Action(allocator), _destination(destination) {}
TransitionAction::~TransitionAction() {}
Action::Type TransitionAction::type() const { return Action::TRANSITION; }

ESB::Error TransitionAction::Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Action **action) {
  ESB::UniqueId destination;
  ESB::Error error = ConfigUtil::FindUniqueId(map, "destination", destination);
  if (ESB_SUCCESS != error) {
    return error;
  }

  *action = new (allocator) TransitionAction(allocator, destination);
  if (!*action) {
    return ESB_OUT_OF_MEMORY;
  }

  return ESB_SUCCESS;
}

SendResponseAction::SendResponseAction(ESB::Allocator &allocator, ESB::UInt16 statusCode, char *reasonPhrase)
    : Action(allocator), _statusCode(statusCode), _reasonPhrase(reasonPhrase) {}

SendResponseAction::~SendResponseAction() {
  if (_reasonPhrase) {
    _allocator.deallocate(_reasonPhrase);
    _reasonPhrase = NULL;
  }
}

Action::Type SendResponseAction::type() const { return Action::SEND_RESPONSE; }

ESB::Error SendResponseAction::Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Action **action) {
  const ESB::AST::Integer *integer = NULL;
  ESB::Error error = ConfigUtil::FindInteger(map, "status_code", &integer);
  if (ESB_SUCCESS != error) {
    return error;
  }

  if (0 > integer->value() || ESB_UINT16_MAX < integer->value()) {
    return ESB_INVALID_FIELD;
  }

  const ESB::AST::String *str = NULL;
  error = ConfigUtil::FindString(map, "reason_phrase", &str);
  char *reasonPhrase = NULL;

  switch (error) {
    case ESB_SUCCESS:
      error = ESB::Duplicate(str->value(), allocator, &reasonPhrase);
      if (ESB_SUCCESS != error) {
        return error;
      }
      break;
    case ESB_MISSING_FIELD:
      reasonPhrase = NULL;  // it's optional
      break;
    default:
      return error;
  }

  Action *newAction = new (allocator) SendResponseAction(allocator, integer->value(), reasonPhrase);
  if (!newAction) {
    if (reasonPhrase) {
      allocator.deallocate(reasonPhrase);
    }
    return ESB_OUT_OF_MEMORY;
  }

  *action = newAction;
  return ESB_SUCCESS;
}

}  // namespace ES
