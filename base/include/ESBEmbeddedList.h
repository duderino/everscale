#ifndef ESB_EMBEDDED_LIST_H
#define ESB_EMBEDDED_LIST_H

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

namespace ESB {

/** @defgroup util Utilities */

/** A doubly linked list of EmbeddedListElements
 *
 *  @ingroup util
 */
class EmbeddedList {
 public:
  /** Constructor.
   */
  EmbeddedList();

  /** Destructor.  No cleanup handlers are called
   */
  virtual ~EmbeddedList();

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
  inline EmbeddedListElement *getFirst() { return _head; }

  /** Get the first element in the list, but do not remove the element from the
   * list.
   *
   * @return The first element in the list or NULL if the list has no elements
   */
  inline const EmbeddedListElement *getFirst() const { return _head; }

  /** Remove and return the first element in the list.  This does not call the
   * element's cleanup handler.
   *
   * @return The first element in the list or NULL if the list has no elements.
   */
  inline EmbeddedListElement *removeFirst() {
    EmbeddedListElement *head = _head;

    if (head) remove(head);

    return head;
  }

  /** Add an element to the front of the list.
   *
   * @param The element to add to the front of the list.
   */
  void addFirst(EmbeddedListElement *element);

  /** Get the last element in the list, but do not remove the element from the
   * list.
   *
   * @return The last element in the list or NULL if the list has no elements
   */
  inline EmbeddedListElement *getLast() { return _tail; }

  /** Get the last element in the list, but do not remove the element from the
   * list.
   *
   * @return The last element in the list or NULL if the list has no elements
   */
  inline const EmbeddedListElement *getLast() const { return _tail; }

  /** Remove and return the last element in the list.  This does not call the
   * element's cleanup handler.
   *
   * @return The last element in the list or NULL if the list has no elements
   */
  inline EmbeddedListElement *removeLast() {
    EmbeddedListElement *tail = _tail;

    if (tail) remove(tail);

    return tail;
  }

  /** Add an element to the end of the list
   *
   * @param element The element to add to the end of the list
   */
  void addLast(EmbeddedListElement *element);

  /** Remove an element from an arbitrary position in the list.  This does not
   * call the element's cleanup handler.
   *
   * @param element the element to remove
   */
  void remove(EmbeddedListElement *element);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

  /** Validate the internal invariants of the list
   *
   * @return true if the list is valid, false otherwise
   */
  bool validate() const;

 private:
  // Disabled
  EmbeddedList(const EmbeddedList &);
  EmbeddedList &operator=(const EmbeddedList &);

  EmbeddedListElement *_head;
  EmbeddedListElement *_tail;
};

}  // namespace ESB

#endif
