#ifndef ESB_AST_DECIMAL_H
#define ESB_AST_DECIMAL_H

#ifndef ESB_AST_SCALAR_H
#include <ASTScalar.h>
#endif

namespace ESB {
namespace AST {

/** A single-value JSON Decimal
 *
 *  @ingroup ast
 */
class Decimal : public Scalar {
 public:
  /** Constructor.
   */
  Decimal(double value, Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~Decimal();

  virtual Type type() const;

  inline int compare(const Decimal &other) const { return _value > other._value ? 1 : _value < other._value ? -1 : 0; }

  inline void setValue(double value) { _value = value; }

  inline double value() const { return _value; }

 private:
  double _value;

  ESB_DEFAULT_FUNCS(Decimal);
};

}  // namespace AST
}  // namespace ESB

#endif
