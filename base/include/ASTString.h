#ifndef ESB_AST_STRING_H
#define ESB_AST_STRING_H

#ifndef ESB_AST_SCALAR_H
#include <ASTScalar.h>
#endif

namespace ESB {
namespace AST {

/** A single-value AST String
 *
 *  @ingroup ast
 */
class String : public Scalar {
 public:
  /** Constructor.
   */
  String(Allocator &allocator = SystemAllocator::Instance(), bool duplicate = true);

  /** Destructor.
   */
  virtual ~String();

  virtual Type type() const;

  Error setValue(const char *buffer, UWord size);

  inline Error setValue(const char *str) { return setValue(str, strlen(str)); }

  int compare(const String &other) const;

  inline const char *value() const { return _value; }

 private:
  const char *_value;
  const bool _duplicate;

  ESB_DEFAULT_FUNCS(String);
};

}  // namespace AST
}  // namespace ESB

#endif
