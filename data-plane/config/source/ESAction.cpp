#ifndef ES_ACTION_H
#include <ESAction.h>
#endif

#ifndef ES_CONFIG_UTIL_H
#include <ESConfigUtil.h>
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
                                          "MARK"};

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
    ESB::Error error = ConfigUtil::StringValue(map, "type", &str);
    if (ESB_SUCCESS != error) {
      return error;
    }

    type = ParseActionType(str->value());
    if (UNKNOWN == type) {
      return ESB_INVALID_FIELD;
    }
  }

  switch (type) {
    default:
      return ESB_NOT_IMPLEMENTED;
  }
}

}  // namespace ES
