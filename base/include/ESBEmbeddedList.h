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
  void clear(bool cleanup = true);

  /** Determine whether the list is empty.  This is O(1)
   *
   * @return true if empty, false otherwise.
   */
  inline bool isEmpty() const { return NULL == _head; }

  /** Determine the number of elements in the list.  This is O(n)
   *
   * @return the number of elements in the list.
   */
  int size() const;

  /** Get the first element in the list, but do not remove the element from the
   * list.
   *
   * @return The first element in the list or NULL if the list has no elements
   */
  inline EmbeddedListElement *first() { return _head; }

  /** Get the first element in the list, but do not remove the element from the
   * list.
   *
   * @return The first element in the list or NULL if the list has no elements
   */
  inline const EmbeddedListElement *first() const { return _head; }

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
  inline EmbeddedListElement *last() { return _tail; }

  /** Get the last element in the list, but do not remove the element from the
   * list.
   *
   * @return The last element in the list or NULL if the list has no elements
   */
  inline const EmbeddedListElement *last() const { return _tail; }

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

  /** Return the element at an index where the first element is at index 0.
   * O(n).
   *
   * @param idx the index
   * @return The element at the index or NULL if there is no element at the
   * index.
   */
  EmbeddedListElement *index(int idx);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, EmbeddedList *list) noexcept { return list; }

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
