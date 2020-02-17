#ifndef ESB_SHARED_EMBEDDED_LIST_H
#define ESB_SHARED_EMBEDDED_LIST_H

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_LOCKABLE_H
#include <ESBLockable.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

#ifndef ESB_READ_SCOPE_LOCK_H
#include <ESBReadScopeLock.h>
#endif

namespace ESB {

/** @defgroup util Utilities */

/** A synchronized doubly linked list of EmbeddedListElements
 *
 *  @ingroup util
 */
class SharedEmbeddedList {
 public:
  /** Constructor.
   */
  SharedEmbeddedList(Lockable *lockable);

  /** Destructor.
   */
  virtual ~SharedEmbeddedList();

  inline bool isEmpty() {
    ReadScopeLock scopeLock(_lockable);

    return _embeddedList.isEmpty();
  }

  inline int length() {
    ReadScopeLock scopeLock(_lockable);

    return _embeddedList.length();
  }

  inline EmbeddedListElement *getFirst() {
    ReadScopeLock scopeLock(_lockable);

    return _embeddedList.getFirst();
  }

  inline EmbeddedListElement *removeFirst() {
    WriteScopeLock scopeLock(_lockable);

    return _embeddedList.removeFirst();
  }

  inline void prepend(EmbeddedListElement *element) {
    WriteScopeLock scopeLock(_lockable);

    _embeddedList.prepend(element);
  }

  inline EmbeddedListElement *getLast() {
    ReadScopeLock scopeLock(_lockable);

    return _embeddedList.getLast();
  }

  inline EmbeddedListElement *removeLast() {
    WriteScopeLock scopeLock(_lockable);

    return _embeddedList.removeLast();
  }

  inline void append(EmbeddedListElement *element) {
    WriteScopeLock scopeLock(_lockable);

    _embeddedList.append(element);
  }

  inline void writeLock() { _lockable->writeAcquire(); }

  inline void writeUnlock() { _lockable->writeRelease(); }

  inline void readLock() { _lockable->readAcquire(); }

  inline void readUnlock() { _lockable->readRelease(); }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation
   * failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  // Disabled
  SharedEmbeddedList(const SharedEmbeddedList &);
  SharedEmbeddedList &operator=(const SharedEmbeddedList &);

  EmbeddedList _embeddedList;
  Mutex _lock;
};

}  // namespace ESB

#endif
