#ifndef ESB_AST_LIST_H
#include <ASTList.h>
#endif

namespace ESB {
namespace AST {

List::List(Allocator &allocator) : Element(allocator) {}

List::~List() { clear(); }

Element::Type List::type() const { return Element::LIST; }

}  // namespace AST
}  // namespace ESB
