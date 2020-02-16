/** @file ESFSharedEmbeddedList.h
 *  @brief A synchronized doubly linked list of ESFEmbeddedListElements
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.2 $
 */

#ifndef ESF_SHARED_EMBEDDED_LIST_H
#define ESF_SHARED_EMBEDDED_LIST_H

#ifndef ESF_EMBEDDED_LIST_H
#include <ESFEmbeddedList.h>
#endif

#ifndef ESF_LOCKABLE_H
#include <ESFLockable.h>
#endif

#ifndef ESF_WRITE_SCOPE_LOCK_H
#include <ESFWriteScopeLock.h>
#endif

#ifndef ESF_READ_SCOPE_LOCK_H
#include <ESFReadScopeLock.h>
#endif

/** @defgroup util Utilities */

/** A synchronized doubly linked list of ESFEmbeddedListElements
 *
 *  @ingroup util
 */
class ESFSharedEmbeddedList {
 public:
  /** Constructor.
   */
  ESFSharedEmbeddedList(ESFLockable *lockable);

  /** Destructor.
   */
  virtual ~ESFSharedEmbeddedList();

  inline bool isEmpty() {
    ESFReadScopeLock scopeLock(_lockable);

    return _embeddedList.isEmpty();
  }

  inline int length() {
    ESFReadScopeLock scopeLock(_lockable);

    return _embeddedList.length();
  }

  inline ESFEmbeddedListElement *getFirst() {
    ESFReadScopeLock scopeLock(_lockable);

    return _embeddedList.getFirst();
  }

  inline ESFEmbeddedListElement *removeFirst() {
    ESFWriteScopeLock scopeLock(_lockable);

    return _embeddedList.removeFirst();
  }

  inline void prepend(ESFEmbeddedListElement *element) {
    ESFWriteScopeLock scopeLock(_lockable);

    _embeddedList.prepend(element);
  }

  inline ESFEmbeddedListElement *getLast() {
    ESFReadScopeLock scopeLock(_lockable);

    return _embeddedList.getLast();
  }

  inline ESFEmbeddedListElement *removeLast() {
    ESFWriteScopeLock scopeLock(_lockable);

    return _embeddedList.removeLast();
  }

  inline void append(ESFEmbeddedListElement *element) {
    ESFWriteScopeLock scopeLock(_lockable);

    _embeddedList.append(element);
  }

  inline void writeLock() { _lockable->writeAcquire(); }

  inline void writeUnlock() { _lockable->writeRelease(); }

  inline void readLock() {
    _lockable->

        /** Placement new.
         *
         *  @param size The size of the object.
         *  @param allocator The source of the object's memory.
         *  @return Memory for the new object or NULL if the memory allocation
         * failed.
         */
        inline void *
        operator new(size_t size, ESFAllocator *allocator) {
      return allocator->allocate(size);
    }

   private:
    // Disabled
    ESFSharedEmbeddedList(const ESFSharedEmbeddedList &);
    ESFSharedEmbeddedList &operator=(const ESFSharedEmbeddedList &);

    ESFEmbeddedList _embeddedList;
    ESFMutex _lock;
  };

#endif /* ! ESF_SHARED_EMBEDDED_LIST_H */
