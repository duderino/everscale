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

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ESB {

/** @defgroup smart_ptr Smart Pointers */

/** Any class that extends ReferenceCount can be used with SmartPointer
 *
 *  @see SmartPointer
 *  @ingroup smart_ptr
 */
class ReferenceCount {
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
  inline void inc() { _refCount.inc(); }

  /** Decrement the reference count.
   */
  inline void dec() { _refCount.dec(); }

  /** Decrement the reference count and return true if new count is zero.
   *
   *    @return true if count is zero after the decrement.
   */
  inline bool decAndTest() {
    bool result = _refCount.decAndTest();
    return result;
  }

  /** Operator new
   *
   *  @param size The size of the object
   *  @return The new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size) noexcept {
    ReferenceCount *object =
        (ReferenceCount *)SystemAllocator::Instance().allocate(size);

    if (object) {
      object->_allocator = &SystemAllocator::Instance();
    }

    return object;
  }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept {
    ReferenceCount *object = (ReferenceCount *)allocator.allocate(size);

    if (object) {
      object->_allocator = &allocator;
    }

    return object;
  }

  /** Operator delete
   *
   *  @param object The object to delete.
   */
  inline void operator delete(void *object) {
    if (((ReferenceCount *)object)->_allocator) {
      ((ReferenceCount *)object)->_allocator->deallocate(object);
    }
  }

 private:
  //  Disabled
  ReferenceCount(const ReferenceCount &);
  ReferenceCount &operator=(const ReferenceCount &);

  SharedInt _refCount;
  Allocator *_allocator;
};

}  // namespace ESB

#endif
