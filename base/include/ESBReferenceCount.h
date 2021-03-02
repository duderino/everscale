#ifndef ESB_REFERENCE_COUNT_H
#define ESB_REFERENCE_COUNT_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

namespace ESB {

/** @defgroup smart_ptr Smart Pointers */

/** Any class that extends ReferenceCount can be used with SmartPointer
 *
 *  @see SmartPointer
 *  @ingroup smart_ptr
 */
class ReferenceCount : public EmbeddedListElement {
  friend class SmartPointer;

 public:
  /** Default constructor.
   */
  ReferenceCount();

  /** Destructor.
   */
  virtual ~ReferenceCount();

  /** Increment the reference count.
   */
  inline int inc() { return _refCount.inc(); }

  /** Decrement the reference count.
   */
  inline int dec() { return _refCount.dec(); }

 private:
  SharedInt _refCount;

  ESB_DISABLE_AUTO_COPY(ReferenceCount);
};

}  // namespace ESB

#endif
