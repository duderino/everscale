#ifndef ESB_JSON_BOOLEAN_H
#define ESB_JSON_BOOLEAN_H

#ifndef ESB_JSON_SCALAR_H
#include <ESBJsonScalar.h>
#endif

namespace ESB {

/** A single-value JSON Boolean
 *
 *  @ingroup json
 */
class JsonBoolean : public JsonScalar {
 public:
  /** Constructor.
   */
  JsonBoolean(bool value, Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~JsonBoolean();

  virtual Type type() const;

  inline int compare(const JsonBoolean &other) const {
    return _value > other._value ? 1 : _value < other._value ? -1 : 0;
  }

  inline void setValue(bool value) { _value = value; }

  inline bool value() const { return _value; }

 private:
  bool _value;

  ESB_DEFAULT_FUNCS(JsonBoolean);
};

}  // namespace ESB

#endif
