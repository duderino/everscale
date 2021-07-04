#ifndef ESB_JSON_INTEGER_H
#include <ESBJsonInteger.h>
#endif

namespace ESB {

JsonInteger::JsonInteger(Int64 value, Allocator &allocator) : JsonScalar(allocator), _value(value) {}

JsonInteger::~JsonInteger() {}

JsonElement::Type JsonInteger::type() const { return INTEGER; }

}  // namespace ESB
