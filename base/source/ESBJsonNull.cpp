#ifndef ESB_JSON_NULL_H
#include <ESBJsonNull.h>
#endif

namespace ESB {

JsonNull::JsonNull(Allocator &allocator) : JsonElement(allocator) {}

JsonNull::~JsonNull() {}

JsonElement::Type JsonNull::type() const { return JsonElement::NIL; }

}  // namespace ESB
