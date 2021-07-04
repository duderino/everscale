#ifndef ESB_JSON_BOOLEAN_H
#include <ESBJsonBoolean.h>
#endif

namespace ESB {

JsonBoolean::JsonBoolean(bool value, Allocator &allocator) : JsonScalar(allocator), _value(value) {}

JsonBoolean::~JsonBoolean() {}

JsonElement::Type JsonBoolean::type() const { return BOOLEAN; }

}  // namespace ESB
