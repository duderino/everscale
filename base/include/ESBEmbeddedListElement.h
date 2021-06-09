#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#define ESB_EMBEDDED_LIST_ELEMENT_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
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

  inline void setPrevious(EmbeddedListElement *previous) { _previous = previous; }

 private:
  EmbeddedListElement *_next;
  EmbeddedListElement *_previous;

  ESB_DISABLE_AUTO_COPY(EmbeddedListElement);
};

}  // namespace ESB

#endif
