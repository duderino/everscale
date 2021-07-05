#ifndef ESB_AST_SCALAR_H
#define ESB_AST_SCALAR_H

#ifndef ESB_AST_ELEMENT_H
#include <ASTElement.h>
#endif

namespace ESB {
namespace AST {

/** An abstract base class for single-valued JSON elements like strings, floats, and integers.
 *
 *  @ingroup json
 */
class Scalar : public Element {
 public:
  /** Constructor.
   */
  Scalar(Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~Scalar();

  ESB_DISABLE_AUTO_COPY(Scalar);
};

}  // namespace AST
}  // namespace ESB

#endif
