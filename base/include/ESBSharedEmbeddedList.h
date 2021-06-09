#ifndef ESB_SHARED_EMBEDDED_LIST_H
#define ESB_SHARED_EMBEDDED_LIST_H

#ifndef ESB_EMBEDDED_ELEMENT_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

namespace ESB {

/** @defgroup util Utilities */

/** A doubly linked list of EmbeddedListElements guarded by a Mutex
 *
 *  @ingroup util
 */
class SharedEmbeddedList {
 public:
  /** Constructor.
   */
  SharedEmbeddedList();

  /** Destructor.  No cleanup handlers are called
   */
  virtual ~SharedEmbeddedList();

  /** Remove all elements from the list.
   *
   *  @param cleanup If true, all elements that have cleanup handlers are
   *      destroyed with their cleanup handlers.  Otherwise the elements
   *      are removed from the list but not destroyed.
   */
  inline void clear(bool cleanup = true) {
    _lock.writeAcquire();
    _list.clear(cleanup);
    _lock.writeRelease();
  }

  /** Determine whether the list is empty.  This is O(1)
   *
   * @return true if empty, false otherwise.
   */
  inline bool isEmpty() const {
    bool result;
    _lock.readAcquire();
    result = _list.isEmpty();
    _lock.readRelease();
    return result;
  }

  /** Determine the number of elements in the list.  This is O(n)
   *
   * @return the number of elements in the list.
   */
  inline int size() const {
    int size;
    _lock.readAcquire();
    size = _list.size();
    _lock.readRelease();
    return size;
  }

  /** Remove and return the first element in the list.  This does not call the
   * element's cleanup handler.
   *
   * @return The first element in the list or NULL if the list has no elements.
   */
  inline EmbeddedListElement *removeFirst() {
    EmbeddedListElement *result;
    _lock.writeAcquire();
    result = _list.removeFirst();
    _lock.writeRelease();
    return result;
  }

  /** Add an element to the front of the list.
   *
   * @param The element to add to the front of the list.
   */
  inline void addFirst(EmbeddedListElement *element) {
    _lock.writeAcquire();
    _list.addFirst(element);
    _lock.writeRelease();
  }

  /** Remove and return the last element in the list.  This does not call the
   * element's cleanup handler.
   *
   * @return The last element in the list or NULL if the list has no elements
   */
  inline EmbeddedListElement *removeLast() {
    EmbeddedListElement *result;
    _lock.writeAcquire();
    result = _list.removeLast();
    _lock.writeRelease();
    return result;
  }

  /** Add an element to the end of the list
   *
   * @param element The element to add to the end of the list
   */
  inline void addLast(EmbeddedListElement *element) {
    _lock.writeAcquire();
    _list.addLast(element);
    _lock.writeRelease();
  }

  /** Remove an element from an arbitrary position in the list.  This does not
   * call the element's cleanup handler.
   *
   * @param element the element to remove
   */
  inline void remove(EmbeddedListElement *element) {
    _lock.writeAcquire();
    _list.remove(element);
    _lock.writeRelease();
  }

  /** Return the element at an index where the first element is at index 0.
   * O(n).
   *
   * @param idx the index
   * @return The element at the index or NULL if there is no element at the
   * index.
   */
  inline EmbeddedListElement *index(int idx) {
    EmbeddedListElement *result;
    _lock.writeAcquire();
    result = _list.index(idx);
    _lock.writeRelease();
    return result;
  }

  /** Validate the internal invariants of the list
   *
   * @return true if the list is valid, false otherwise
   */
  inline bool validate() const {
    bool result;
    _lock.readAcquire();
    result = _list.validate();
    _lock.readRelease();
    return result;
  }

 private:
  EmbeddedList _list;
  mutable Mutex _lock;

  ESB_DEFAULT_FUNCS(SharedEmbeddedList);
};

}  // namespace ESB

#endif
