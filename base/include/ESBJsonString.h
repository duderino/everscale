#ifndef ESB_JSON_STRING_H
#define ESB_JSON_STRING_H

#ifndef ESB_JSON_SCALAR_H
#include <ESBJsonScalar.h>
#endif

namespace ESB {

/** A single-value JSON String
 *
 *  @ingroup json
 */
class JsonString : public JsonScalar {
 public:
  /** Constructor.
   */
  JsonString(Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~JsonString();

  virtual Type type() const;

  Error setValue(const char *buffer, UWord size);

  inline Error setValue(const char *str) { return setValue(str, strlen(str)); }

  int compare(const JsonString &other) const;

  inline const char *value() const { return _value; }

 private:
  char *_value;

  ESB_DEFAULT_FUNCS(JsonString);
};

}  // namespace ESB

#endif
