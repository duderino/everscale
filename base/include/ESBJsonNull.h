#ifndef ESB_JSON_NULL_H
#define ESB_JSON_NULL_H

#ifndef ESB_JSON_ELEMENT_H
#include <ESBJsonElement.h>
#endif

namespace ESB {

/** A JSON Null.
 *
 *  @ingroup json
 */
class JsonNull : public JsonElement {
 public:
  /** Constructor.
   */
  JsonNull(Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~JsonNull();

  virtual Type type() const;

  ESB_DEFAULT_FUNCS(JsonNull);
};

}  // namespace ESB

#endif
