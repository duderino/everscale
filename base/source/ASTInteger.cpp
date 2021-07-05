#ifndef ESB_AST_INTEGER_H
#include <ASTInteger.h>
#endif

namespace ESB {
namespace AST {

Integer::Integer(Int64 value, Allocator &allocator) : Scalar(allocator), _value(value) {}

Integer::~Integer() {}

Element::Type Integer::type() const { return INTEGER; }

}  // namespace AST
}  // namespace ESB
