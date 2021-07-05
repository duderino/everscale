#ifndef ESB_AST_BOOLEAN_H
#include <ASTBoolean.h>
#endif

namespace ESB {
namespace AST {

Boolean::Boolean(bool value, Allocator &allocator) : Scalar(allocator), _value(value) {}

Boolean::~Boolean() {}

Element::Type Boolean::type() const { return BOOLEAN; }

}  // namespace AST
}  // namespace ESB
