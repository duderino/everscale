#ifndef ESB_JSON_ELEMENT_H
#define ESB_JSON_ELEMENT_H

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ESB {

/** An abstract base class for all JSON elements.
 *
 *  @defgroup json
 *  @ingroup json
 */
class JsonElement : public EmbeddedListElement {
 public:
  enum Type { NIL = 0, MAP = 1, LIST = 2, STRING = 3, INTEGER = 4, DECIMAL = 5, BOOLEAN = 6 };

  /** Constructor.
   */
  JsonElement(Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~JsonElement();

  virtual CleanupHandler *cleanupHandler();

  virtual Type type() const = 0;

 protected:
  Allocator &_allocator;

 private:
  ESB_DISABLE_AUTO_COPY(JsonElement);
};

}  // namespace ESB

#endif
