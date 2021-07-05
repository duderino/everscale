#ifndef ESB_AST_ELEMENT_H
#define ESB_AST_ELEMENT_H

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ESB {
namespace AST {

/** An abstract base class for all JSON elements.
 *
 *  @defgroup ast
 *  @ingroup ast
 */
class Element : public EmbeddedListElement {
 public:
  enum Type { NIL = 0, MAP = 1, LIST = 2, STRING = 3, INTEGER = 4, DECIMAL = 5, BOOLEAN = 6 };

  /** Constructor.
   */
  Element(Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~Element();

  virtual CleanupHandler *cleanupHandler();

  virtual Type type() const = 0;

 protected:
  Allocator &_allocator;

 private:
  ESB_DISABLE_AUTO_COPY(Element);
};

}  // namespace AST
}  // namespace ESB

#endif
