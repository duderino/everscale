#ifndef ESB_JSON_DECIMAL_H
#include <ESBJsonDecimal.h>
#endif

namespace ESB {
JsonDecimal::JsonDecimal(double value, Allocator &allocator) : JsonScalar(allocator), _value(value) {}

JsonDecimal::~JsonDecimal() {}

JsonElement::Type JsonDecimal::type() const { return DECIMAL; }

}  // namespace ESB
