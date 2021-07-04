#ifndef ESB_JSON_LIST_H
#include <ESBJsonList.h>
#endif

namespace ESB {

JsonList::JsonList(Allocator &allocator) : JsonElement(allocator) {}

JsonList::~JsonList() { clear(); }

JsonElement::Type JsonList::type() const { return JsonElement::LIST; }

}  // namespace ESB
