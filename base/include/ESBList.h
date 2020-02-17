#ifndef ESB_LIST_H
#define ESB_LIST_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_LOCKABLE_H
#include <ESBLockable.h>
#endif

namespace ESB {

/** @todo several List operations require const versions */

/** This class is the internal node used by List and ListIterator.  It
 *  should not be used directly by client code.
 */
typedef struct ListNode {
  struct ListNode *_prev;
  struct ListNode *_next;
  void *_value;
} ListNode;

class List;

/** ListIterator allows iteration through the List class.
 *  ListIterators follow the same iterator invalidation rules as
 *  STL list iterators.  That is, changes to a List instance do not
 *  invalidate any existing iterators unless that change deleted the very node
 *  that the iterator points to.
 *
 *  @ingroup collection
 */
class ListIterator {
  friend class List;

 public:
  /** Constructor.
   */
  ListIterator();

  /** Copy constructor.
   *
   *  @param iterator The iterator to copy.
   */
  ListIterator(const ListIterator &iterator);

  /** Destructor. */
  virtual ~ListIterator();

  /** Assignment operator.
   *
   *  @param iterator The iterator to copy.
   */
  inline ListIterator &operator=(const ListIterator &iterator) {
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
  inline ListIterator getNext() {
    ListIterator iterator(_node ? _node->_next : 0);

    return iterator;
  }

  /** Get an iterator for the element before the element pointed to by this
   *  iterator.  O(1).
   *
   *  @return The iterator.  If there is no prior element, the iterator's
   *      getValue() method will return NULL.
   */
  inline ListIterator getPrevious() {
    ListIterator iterator(_node ? _node->_prev : 0);

    return iterator;
  }

  /** Pre-increment operator.  O(1).  Point this iterator at the next
   *  element in the list.  If there is no next element, the iterator will
   *  be set to null (i.e., its isNull method will return true and its
   *  getValue method will return NULL).
   *
   *  @return The iterator itself
   */
  inline ListIterator &operator++() {
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
  inline ListIterator operator++(int) {
    ListIterator it(_node);

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
  inline ListIterator &operator--() {
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
  inline ListIterator operator--(int) {
    ListIterator it(_node);

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
  ListIterator(ListNode *node);

  ListNode *_node;
};

/** List is a doubly linked list that supports most list operations, allows
 *  custom memory allocation, and supports synchronization.
 *  <p>
 *  The values inserted into List instances are not allowed to be NULL.
 *  Up to 4,294,967,295 elements can be inserted into a List instance
 *  assuming it can always allocate enough memory for its internal nodes.
 *  </p>
 *
 *  @ingroup collection
 */
class List : public Lockable {
  friend class ListIterator;

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
  List(Allocator *allocator, Lockable *lockable);

  /** Destructor. */
  virtual ~List();

  /** Insert an element before the head of the list.  O(1).
   *
   *  @param element The element to insert.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error pushFront(void *element);

  /** Insert an element after the tail of the list.  O(1).
   *
   *  @param element The element to insert.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error pushBack(void *element);

  /** Get the element at the head of the list.  This does not remove the
   *  element from the list.  O(1).
   *
   *  @return The head of the list if the list was not empty, NULL otherwise.
   */
  void *getFront();

  /** Remove the element at the head of the list.  O(1).
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.  If
   *      the list is empty, ESB_SUCCESS will be returned.
   */
  Error popFront();

  /** Get the element at the tail of the list.  This does not remove the
   *  element from the list.  O(1).
   *
   *  @return The tail of the list if the list was not empty, NULL otherwise.
   */
  void *getBack();

  /** Remove the element at the tail of the list.  O(1).
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.  If
   *      the list is empty, ESB_SUCCESS will be returned.
   */
  Error popBack();

  /** Remove all elements from the list.  O(n).
   *  <p>
   *  This will only deallocate memory used by the list's internal nodes.
   *  </p>
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error clear();

  /** Get an iterator pointing to the head of the list.  If the list is
   *  empty, the getValue() method of the iterator will return NULL.  O(1).
   *  <p>
   *  Note that the list itself must be synchronized whenever any of the
   *  iterators methods are called if the list is shared by multiple threads.
   *  </p>
   *  @return An iterator pointing to the first element in the list.
   */
  ListIterator getFrontIterator();

  /** Get an iterator pointing to the tail of the list.  If the list is
   *  empty, the getValue() method of the iterator will return NULL.  O(1).
   *  <p>
   *  Note that the list itself must be synchronized whenever any of the
   *  iterators methods are called if the list is shared by multiple threads.
   *  </p>
   *  @return An iterator pointing to the last element in the list.
   */
  ListIterator getBackIterator();

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
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error erase(ListIterator *iterator);

  /** Get the current size of the list.  O(1).
   *
   *  @return The current size of the list.
   */
  UInt32 getSize() const;

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
  static Size GetAllocationSize();

  /** Block the calling thread until write access is granted.
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error writeAcquire();

  /** Block the calling thread until read access is granted.
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error readAcquire();

  /** Attempt to gain write access, returning immediately if access could not
   *  be granted.
   *
   *  @return ESB_SUCCESS if access was granted, ESB_AGAIN if access could
   *      not be immediately granted, or another error code if an error
   *      occurred.
   */
  virtual Error writeAttempt();

  /** Attempt to gain read access, returning immediately if access could not
   *  be granted.
   *
   *  @return ESB_SUCCESS if access was granted, ESB_AGAIN if access could
   *      not be immediately granted, or another error code if an error
   *      occurred.
   */
  virtual Error readAttempt();

  /** Release the lock after write access was granted.
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error writeRelease();

  /** Release the lock after read access was granted.
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error readRelease();

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  //  Disabled
  List(const List &);
  List &operator=(const List &);

  Error deleteNode(ListNode *node);

  UInt32 _size;

  ListNode *_head;
  ListNode *_tail;

  Allocator *_allocator;
  Lockable *_lockable;
};

}  // namespace ESB

#endif
