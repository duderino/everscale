#ifndef ESB_AST_DECIMAL_H
#include <ASTDecimal.h>
#endif

namespace ESB {
namespace AST {

Decimal::Decimal(double value, Allocator &allocator) : Scalar(allocator), _value(value) {}

Decimal::~Decimal() {}

Element::Type Decimal::type() const { return DECIMAL; }

}  // namespace AST
}  // namespace ESB
