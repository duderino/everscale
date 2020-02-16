/** @file ESFList.h
 *  @brief A doubly-linked list that stores its values as void pointers
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_LIST_H
#define ESF_LIST_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_LOCKABLE_H
#include <ESFLockable.h>
#endif

/** @todo several ESFList operations require const versions */

/** This class is the internal node used by ESFList and ESFListIterator.  It
 *  should not be used directly by client code.
 */
typedef struct ESFListNode {
  struct ESFListNode *_prev;
  struct ESFListNode *_next;
  void *_value;
} ESFListNode;

class ESFList;

/** ESFListIterator allows iteration through the ESFList class.
 *  ESFListIterators follow the same iterator invalidation rules as
 *  STL list iterators.  That is, changes to a ESFList instance do not
 *  invalidate any existing iterators unless that change deleted the very node
 *  that the iterator points to.
 *
 *  @ingroup collection
 */
class ESFListIterator {
  friend class ESFList;

 public:
  /** Constructor.
   */
  ESFListIterator();

  /** Copy constructor.
   *
   *  @param iterator The iterator to copy.
   */
  ESFListIterator(const ESFListIterator &iterator);

  /** Destructor. */
  virtual ~ESFListIterator();

  /** Assignment operator.
   *
   *  @param iterator The iterator to copy.
   */
  inline ESFListIterator &operator=(const ESFListIterator &iterator) {
    _node = iterator._node;

    return *this;
  }

  /** Determine whether there is another element after the element pointed
   *  to by this iterator.  O(1).
   *
   *  @return true if there is, false otherwise.
   */
  inline bool hasNext() const {
    if (!_node) return false;

    return 0 != _node->_next;
  }

  /** Determine whether there is another element before the element pointed
   *  to by this iterator.  O(1).
   *
   *  @return true if there is, false otherwise.
   */
  inline bool hasPrevious() const {
    if (!_node) return false;

    return 0 != _node->_prev;
  }

  /** Get an iterator for the element after the element pointed to by this
   *  iterator.  O(1).
   *
   *  @return The iterator.  If there is no following element, the iterator's
   *      getValue() method will return NULL.
   */
  inline ESFListIterator getNext() {
    ESFListIterator iterator(_node ? _node->_next : 0);

    return iterator;
  }

  /** Get an iterator for the element before the element pointed to by this
   *  iterator.  O(1).
   *
   *  @return The iterator.  If there is no prior element, the iterator's
   *      getValue() method will return NULL.
   */
  inline ESFListIterator getPrevious() {
    ESFListIterator iterator(_node ? _node->_prev : 0);

    return iterator;
  }

  /** Pre-increment operator.  O(1).  Point this iterator at the next
   *  element in the list.  If there is no next element, the iterator will
   *  be set to null (i.e., its isNull method will return true and its
   *  getValue method will return NULL).
   *
   *  @return The iterator itself
   */
  inline ESFListIterator &operator++() {
    _node = _node ? _node->_next : 0;

    return *this;
  }

  /** Post-increment operator.  O(1).  Point this iterator at the next
   *  element in the list.  If there is no next element, the iterator will
   *  be set to null (i.e., its isNull method will return true and its
   *  getValue method will return NULL).
   *
   *  @return A new iterator pointing to the key/value pair before the
   *      increment operation.
   */
  inline ESFListIterator operator++(int) {
    ESFListIterator it(_node);

    _node = _node ? _node->_next : 0;

    return it;
  }

  /** Pre-decrement operator.  O(1).  Point this iterator at the previous
   *  element in the list.  If there is no previous element, the iterator will
   *  be set to null (i.e., its isNull method will return true and its
   *  getValue method will return NULL).
   *
   *  @return The iterator itself
   */
  inline ESFListIterator &operator--() {
    _node = _node ? _node->_prev : 0;

    return *this;
  }

  /** Post-decrement operator.  O(1).  Point this iterator at the previous
   *  element in the list.  If there is no previous element, the iterator will
   *  be set to null (i.e., its isNull method will return true and its
   *  getValue method will return NULL).
   *
   *  @return A new iterator pointing to the key/value pair before the
   *      increment operation.
   */
  inline ESFListIterator operator--(int) {
    ESFListIterator it(_node);

    _node = _node ? _node->_prev : 0;

    return it;
  }

  /** Get the element this iterator points to.  O(1).
   *
   *  @return The element or NULL if this iterator does not point to any
   *      element.
   */
  inline void *getValue() {
    if (!_node) return 0;

    return _node->_value;
  }

  /** Set the value that this iterator points to.  O(1).  Note that this
   *  will not free any resources used by the old value.
   *
   *  @param value The new value for this element.  Cannot be NULL.
   */
  inline void setValue(void *value) {
    if (!_node || !value) return;

    _node->_value = value;
  }

  /** Determine whether the element the iterator points to is NULL. O(1).
   *
   *  @return true if the element the iterator points to is NULL, false
   *      otherwise.
   */
  inline bool isNull() const { return 0 == _node; }

 private:
  /** Constructor.
   *
   *  @param node The element that this iterator initially points to.
   */
  ESFListIterator(ESFListNode *node);

  ESFListNode *_node;
};

/** ESFList is a doubly linked list that supports most list operations, allows
 *  custom memory allocation, and supports synchronization.
 *  <p>
 *  The values inserted into ESFList instances are not allowed to be NULL.
 *  Up to 4,294,967,295 elements can be inserted into a ESFList instance
 *  assuming it can always allocate enough memory for its internal nodes.
 *  </p>
 *
 *  @ingroup collection
 */
class ESFList : public ESFLockable {
  friend class ESFListIterator;

 public:
  /** Constructor.
   *
   *  @param allocator The allocator that the list will use to create its
   *      internal nodes.
   *  @param lockable A lock that will be used to synchronize the list.  All
   *      of the list's synchronizing methods will forward to this object.
   *  @see GetAllocationSize to determine how much memory the list will
   *      allocate for every internal node it creates.  This is useful for
   *      constructing fixed length allocators.
   */
  ESFList(ESFAllocator *allocator, ESFLockable *lockable);

  /** Destructor. */
  virtual ~ESFList();

  /** Insert an element before the head of the list.  O(1).
   *
   *  @param element The element to insert.
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  ESFError pushFront(void *element);

  /** Insert an element after the tail of the list.  O(1).
   *
   *  @param element The element to insert.
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  ESFError pushBack(void *element);

  /** Get the element at the head of the list.  This does not remove the
   *  element from the list.  O(1).
   *
   *  @return The head of the list if the list was not empty, NULL otherwise.
   */
  void *getFront();

  /** Remove the element at the head of the list.  O(1).
   *
   *  @return ESF_SUCCESS if successful, another error code otherwise.  If
   *      the list is empty, ESF_SUCCESS will be returned.
   */
  ESFError popFront();

  /** Get the element at the tail of the list.  This does not remove the
   *  element from the list.  O(1).
   *
   *  @return The tail of the list if the list was not empty, NULL otherwise.
   */
  void *getBack();

  /** Remove the element at the tail of the list.  O(1).
   *
   *  @return ESF_SUCCESS if successful, another error code otherwise.  If
   *      the list is empty, ESF_SUCCESS will be returned.
   */
  ESFError popBack();

  /** Remove all elements from the list.  O(n).
   *  <p>
   *  This will only deallocate memory used by the list's internal nodes.
   *  </p>
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  ESFError clear();

  /** Get an iterator pointing to the head of the list.  If the list is
   *  empty, the getValue() method of the iterator will return NULL.  O(1).
   *  <p>
   *  Note that the list itself must be synchronized whenever any of the
   *  iterators methods are called if the list is shared by multiple threads.
   *  </p>
   *  @return An iterator pointing to the first element in the list.
   */
  ESFListIterator getFrontIterator();

  /** Get an iterator pointing to the tail of the list.  If the list is
   *  empty, the getValue() method of the iterator will return NULL.  O(1).
   *  <p>
   *  Note that the list itself must be synchronized whenever any of the
   *  iterators methods are called if the list is shared by multiple threads.
   *  </p>
   *  @return An iterator pointing to the last element in the list.
   */
  ESFListIterator getBackIterator();

  /** Remove the element pointed to by this iterator from the list.  Note
   *  that the iterator will be invalidated in the process.  In fact, all
   *  iterators that point to the element pointed to by this iterator will
   *  be invalidated by this method and there are no checks to prevent
   *  invalidated iterators from being used.  Using invalidated iterators
   *  can corrupt memory.  O(1).
   *  <p>
   *  This will only deallocate memory used by the list's internal nodes.
   *  </p>
   *
   *  @param iterator The iterator that points to the element to remove from
   *      the list.
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  ESFError erase(ESFListIterator *iterator);

  /** Get the current size of the list.  O(1).
   *
   *  @return The current size of the list.
   */
  ESFUInt32 getSize() const;

  /** Determine whether the list is empty.  O(1).
   *
   *  @return true if the list is empty, false otherwise.
   */
  bool isEmpty() const;

  /** Get the size in bytes of the list's interal nodes.  This is the amount
   *  of memory that the list will request from the allocator for every node
   *  it creates.  This is useful for setting the block size of fixed length
   *  allocators assigned assigned to this list.
   *
   *  @return The size in bytes of the list's internal nodes.
   */
  static ESFSize GetAllocationSize();

  /** Block the calling thread until write access is granted.
   *
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  virtual ESFError writeAcquire();

  /** Block the calling thread until read access is granted.
   *
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  virtual ESFError readAcquire();

  /** Attempt to gain write access, returning immediately if access could not
   *  be granted.
   *
   *  @return ESF_SUCCESS if access was granted, ESF_AGAIN if access could
   *      not be immediately granted, or another error code if an error
   *      occurred.
   */
  virtual ESFError writeAttempt();

  /** Attempt to gain read access, returning immediately if access could not
   *  be granted.
   *
   *  @return ESF_SUCCESS if access was granted, ESF_AGAIN if access could
   *      not be immediately granted, or another error code if an error
   *      occurred.
   */
  virtual ESFError readAttempt();

  /** Release the lock after write access was granted.
   *
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  virtual ESFError writeRelease();

  /** Release the lock after read access was granted.
   *
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  virtual ESFError readRelease();

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  //  Disabled
  ESFList(const ESFList &);
  ESFList &operator=(const ESFList &);

  ESFError deleteNode(ESFListNode *node);

  ESFUInt32 _size;

  ESFListNode *_head;
  ESFListNode *_tail;

  ESFAllocator *_allocator;
  ESFLockable *_lockable;
};

#endif /* ! ESF_LIST_H */
