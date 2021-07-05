#ifndef ESB_AST_ELEMENT_H
#include <ASTElement.h>
#endif

namespace ESB {
namespace AST {

Element::Element(Allocator& allocator) : EmbeddedListElement(), _allocator(allocator) {}

Element::~Element() {}

CleanupHandler* Element::cleanupHandler() { return &_allocator.cleanupHandler(); }

}  // namespace AST
}  // namespace ESB
