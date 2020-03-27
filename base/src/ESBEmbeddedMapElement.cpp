#ifndef ESB_EMBEDDED_MAP_ELEMENT_H
#include <ESBEmbeddedMapElement.h>
#endif

namespace ESB {
EmbeddedMapElement::EmbeddedMapElement() : EmbeddedListElement() {}

EmbeddedMapElement::~EmbeddedMapElement() {}

const void *EmbeddedMapElement::key() const { return NULL; }
}  // namespace ESB
