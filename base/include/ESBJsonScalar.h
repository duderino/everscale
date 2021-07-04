#ifndef ESB_JSON_SCALAR_H
#define ESB_JSON_SCALAR_H

#ifndef ESB_JSON_ELEMENT_H
#include <ESBJsonElement.h>
#endif

namespace ESB {

/** An abstract base class for single-valued JSON elements like strings, floats, and integers.
 *
 *  @ingroup json
 */
class JsonScalar : public JsonElement {
 public:
  /** Constructor.
   */
  JsonScalar(Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~JsonScalar();

  ESB_DISABLE_AUTO_COPY(JsonScalar);
};

}  // namespace ESB

#endif
