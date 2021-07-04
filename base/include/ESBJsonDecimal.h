#ifndef ESB_JSON_DECIMAL_H
#define ESB_JSON_DECIMAL_H

#ifndef ESB_JSON_SCALAR_H
#include <ESBJsonScalar.h>
#endif

namespace ESB {

/** A single-value JSON Decimal
 *
 *  @ingroup json
 */
class JsonDecimal : public JsonScalar {
 public:
  /** Constructor.
   */
  JsonDecimal(double value, Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~JsonDecimal();

  virtual Type type() const;

  inline int compare(const JsonDecimal &other) const {
    return _value > other._value ? 1 : _value < other._value ? -1 : 0;
  }

  inline void setValue(double value) { _value = value; }

  inline double value() const { return _value; }

 private:
  double _value;

  ESB_DEFAULT_FUNCS(JsonDecimal);
};

}  // namespace ESB

#endif
