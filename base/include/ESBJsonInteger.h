#ifndef ESB_JSON_INTEGER_H
#define ESB_JSON_INTEGER_H

#ifndef ESB_JSON_SCALAR_H
#include <ESBJsonScalar.h>
#endif

namespace ESB {

/** A single-value JSON Integer
 *
 *  @ingroup json
 */
class JsonInteger : public JsonScalar {
 public:
  /** Constructor.
   */
  JsonInteger(Int64 value, Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~JsonInteger();

  virtual Type type() const;

  inline int compare(const JsonInteger &other) const {
    return _value > other._value ? 1 : _value < other._value ? -1 : 0;
  }

  inline void setValue(Int64 value) { _value = value; }

  inline Int64 value() const { return _value; }

 private:
  Int64 _value;

  ESB_DEFAULT_FUNCS(JsonInteger);
};

}  // namespace ESB

#endif
