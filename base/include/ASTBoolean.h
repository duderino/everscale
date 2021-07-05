#ifndef ESB_AST_BOOLEAN_H
#define ESB_AST_BOOLEAN_H

#ifndef ESB_AST_SCALAR_H
#include <ASTScalar.h>
#endif

namespace ESB {
namespace AST {

/** A single-value AST Boolean
 *
 *  @ingroup ast
 */
class Boolean : public Scalar {
 public:
  /** Constructor.
   */
  Boolean(bool value, Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~Boolean();

  virtual Type type() const;

  inline int compare(const Boolean &other) const { return _value > other._value ? 1 : _value < other._value ? -1 : 0; }

  inline void setValue(bool value) { _value = value; }

  inline bool value() const { return _value; }

 private:
  bool _value;

  ESB_DEFAULT_FUNCS(Boolean);
};

}  // namespace AST
}  // namespace ESB

#endif
