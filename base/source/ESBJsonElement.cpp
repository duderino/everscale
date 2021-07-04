#ifndef ESB_JSON_ELEMENT_H
#include <ESBJsonElement.h>
#endif

namespace ESB {
JsonElement::JsonElement(Allocator& allocator) : EmbeddedListElement(), _allocator(allocator) {}

JsonElement::~JsonElement() {}

CleanupHandler* JsonElement::cleanupHandler() { return &_allocator.cleanupHandler(); }

}  // namespace ESB
