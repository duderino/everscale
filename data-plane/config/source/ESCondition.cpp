#ifndef ES_CONDITION_H
#include <ESCondition.h>
#endif

#ifndef ES_CONFIG_UTIL_H
#include <ESConfigUtil.h>
#endif

namespace ES {

// Must be in same order as ConditionType
static const char *ConditionTypeStrings[] = {"",           "ACQUIRE_SLOT", "PROPERTY_MATCH", "RESULT_MATCH",
                                             "MARK_MATCH", "WAF_MATCH"};

static Condition::Type ParseConditionType(const char *str) {
  if (!str) {
    return Condition::UNKNOWN;
  }

  for (int i = 0; i < sizeof(ConditionTypeStrings) / sizeof(char *); ++i) {
    if (0 == strcasecmp(str, ConditionTypeStrings[i])) {
      return (Condition::Type)i;
    }
  }

  return Condition::UNKNOWN;
}

Condition::Condition(ESB::Allocator &allocator) : _allocator(allocator) {}

Condition::~Condition() {}

ESB::CleanupHandler *Condition::cleanupHandler() { return &_allocator.cleanupHandler(); }

ESB::Error Condition::Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Condition **condition) {
  if (!condition) {
    return ESB_NULL_POINTER;
  }

  Type type = UNKNOWN;

  {
    const ESB::AST::String *str = NULL;
    ESB::Error error = ConfigUtil::StringValue(map, "type", &str);
    if (ESB_SUCCESS != error) {
      return error;
    }

    type = ParseConditionType(str->value());
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
