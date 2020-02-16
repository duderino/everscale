/** @file ESFEmbeddedList.h
 *  @brief A doubly linked list of ESFEmbeddedListElements
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

#ifndef ESF_EMBEDDED_LIST_H
#define ESF_EMBEDDED_LIST_H

#ifndef ESF_EMBEDDED_LIST_ELEMENT_H
#include <ESFEmbeddedListElement.h>
#endif

/** @defgroup util Utilities */

/** A doubly linked list of ESFEmbeddedListElements
 *
 *  @ingroup util
 */
class ESFEmbeddedList {
 public:
  /** Constructor.
   */
  ESFEmbeddedList();

  /** Destructor.  No cleanup handlers are called
   */
  virtual ~ESFEmbeddedList();

  /** Remove all elements from the list.
   *
   *  @param cleanup If true, all elements that have cleanup handlers are
   *      destroyed with their cleanup handlers.  Otherwise the elements
   *      are removed from the list but not destroyed.
   */
  void clear(bool cleanup);

  /** Determine whether the list is empty.  This is O(1)
   *
   * @return true if empty, false otherwise.
   */
  inline bool isEmpty() const { return 0 == _head; }

  /** Determine the number of elements in the list.  This is O(n)
   *
   * @return the number of elements in the list.
   */
  int length() const;

  /** Get the first element in the list, but do not remove the element from the
   * list.
   *
   * @return The first element in the list or NULL if the list has no elements
   */
  inline ESFEmbeddedListElement *getFirst() { return _head; }

  /** Get the first element in the list, but do not remove the element from the
   * list.
   *
   * @return The first element in the list or NULL if the list has no elements
   */
  inline const ESFEmbeddedListElement *getFirst() const { return _head; }

  /** Remove and return the first element in the list.  This does not call the
   * element's cleanup handler.
   *
   * @return The first element in the list or NULL if the list has no elements.
   */
  inline ESFEmbeddedListElement *removeFirst() {
    ESFEmbeddedListElement *head = _head;

    if (head) remove(head);

    return head;
  }

  /** Add an element to the front of the list.
   *
   * @param The element to add to the front of the list.
   */
  void addFirst(ESFEmbeddedListElement *element);

  /** Get the last element in the list, but do not remove the element from the
   * list.
   *
   * @return The last element in the list or NULL if the list has no elements
   */
  inline ESFEmbeddedListElement *getLast() { return _tail; }

  /** Get the last element in the list, but do not remove the element from the
   * list.
   *
   * @return The last element in the list or NULL if the list has no elements
   */
  inline const ESFEmbeddedListElement *getLast() const { return _tail; }

  /** Remove and return the last element in the list.  This does not call the
   * element's cleanup handler.
   *
   * @return The last element in the list or NULL if the list has no elements
   */
  inline ESFEmbeddedListElement *removeLast() {
    ESFEmbeddedListElement *tail = _tail;

    if (tail) remove(tail);

    return tail;
  }

  /** Add an element to the end of the list
   *
   * @param element The element to add to the end of the list
   */
  void addLast(ESFEmbeddedListElement *element);

  /** Remove an element from an arbitrary position in the list.  This does not
   * call the element's cleanup handler.
   *
   * @param element the element to remove
   */
  void remove(ESFEmbeddedListElement *element);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

  /** Validate the internal invariants of the list
   *
   * @return true if the list is valid, false otherwise
   */
  bool validate() const;

 private:
  // Disabled
  ESFEmbeddedList(const ESFEmbeddedList &);
  ESFEmbeddedList &operator=(const ESFEmbeddedList &);

  ESFEmbeddedListElement *_head;
  ESFEmbeddedListElement *_tail;
};

#endif /* ! ESF_EMBEDDED_LIST_H */
