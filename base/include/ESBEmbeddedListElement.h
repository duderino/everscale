#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#define ESB_EMBEDDED_LIST_ELEMENT_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_OBJECT_H
#include <ESBObject.h>
#endif

#ifndef ESB_CLEANUP_HANDLER_H
#include <ESBCleanupHandler.h>
#endif

namespace ESB {

/** An element that can be stored in a EmbeddedList
 *
 *  @ingroup collection
 */
class EmbeddedListElement : public Object {
 public:
  /** Constructor.
   */
  EmbeddedListElement();

  /** Destructor.
   */
  virtual ~EmbeddedListElement();

  /** Return an optional handler that can destroy the element.
   *
   * @return A handler to destroy the element or NULL if the element should not
   * be destroyed.
   */
  virtual CleanupHandler *cleanupHandler() = 0;

  inline EmbeddedListElement *next() { return _next; }

  inline const EmbeddedListElement *next() const { return _next; }

  inline void setNext(EmbeddedListElement *next) { _next = next; }

  inline EmbeddedListElement *previous() { return _previous; }

  inline const EmbeddedListElement *previous() const { return _previous; }

  inline void setPrevious(EmbeddedListElement *previous) {
    _previous = previous;
  }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 private:
  // Disabled
  EmbeddedListElement(const EmbeddedListElement &);
  EmbeddedListElement &operator=(const EmbeddedListElement &);

  EmbeddedListElement *_next;
  EmbeddedListElement *_previous;
};

}  // namespace ESB

#endif
