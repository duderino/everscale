#ifndef ESB_AST_INTEGER_H
#define ESB_AST_INTEGER_H

#ifndef ESB_AST_SCALAR_H
#include <ASTScalar.h>
#endif

namespace ESB {
namespace AST {

/** A single-value AST Integer
 *
 *  @ingroup ast
 */
class Integer : public Scalar {
 public:
  /** Constructor.
   */
  Integer(Int64 value, Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~Integer();

  virtual Type type() const;

  inline int compare(const Integer &other) const { return _value > other._value ? 1 : _value < other._value ? -1 : 0; }

  inline void setValue(Int64 value) { _value = value; }

  inline Int64 value() const { return _value; }

 private:
  Int64 _value;

  ESB_DEFAULT_FUNCS(Integer);
};

}  // namespace AST
}  // namespace ESB

#endif
