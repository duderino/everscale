#ifndef ESB_AST_NULL_H
#define ESB_AST_NULL_H

#ifndef ESB_AST_ELEMENT_H
#include <ASTElement.h>
#endif

namespace ESB {
namespace AST {

/** An AST Null element.
 *
 *  @ingroup ast
 */
class Null : public Element {
 public:
  /** Constructor.
   */
  Null(Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~Null();

  virtual Type type() const;

  ESB_DEFAULT_FUNCS(Null);
};

}  // namespace AST
}  // namespace ESB

#endif
