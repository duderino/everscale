#ifndef ESB_AST_NULL_H
#include <ASTNull.h>
#endif

namespace ESB {
namespace AST {

Null::Null(Allocator &allocator) : Element(allocator) {}

Null::~Null() {}

Element::Type Null::type() const { return Element::NIL; }

}  // namespace AST
}  // namespace ESB
