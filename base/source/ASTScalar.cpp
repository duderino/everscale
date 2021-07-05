#ifndef ESB_AST_SCALAR_H
#include <ASTScalar.h>
#endif

namespace ESB {
namespace AST {

Scalar::Scalar(Allocator &allocator) : Element(allocator) {}

Scalar::~Scalar() {}

}  // namespace AST
}  // namespace ESB
