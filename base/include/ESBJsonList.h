#ifndef ESB_JSON_LIST_H
#define ESB_JSON_LIST_H

#ifndef ESB_JSON_ELEMENT_H
#include <ESBJsonElement.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

namespace ESB {

/** A list of JSON elements
 *
 *  @ingroup json
 */
class JsonList : public JsonElement {
 public:
  /** Constructor.
   */
  JsonList(Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~JsonList();

  virtual Type type() const;

  /** Remove all elements from the list.
   *
   */
  inline void clear() { _list.clear(); }

  /** Determine whether the list is empty.  This is O(1)
   *
   * @return true if empty, false otherwise.
   */
  inline bool isEmpty() const { return _list.isEmpty(); }

  /** Determine the number of elements in the list.  This is O(n)
   *
   * @return the number of elements in the list.
   */
  inline int size() const { return _list.size(); }

  /** Get the first element in the list, but do not remove the element from the list.
   *
   * @return The first element in the list or NULL if the list has no elements
   */
  inline JsonElement *first() { return (JsonElement *)_list.first(); }

  /** Get the first element in the list, but do not remove the element from the list.
   *
   * @return The first element in the list or NULL if the list has no elements
   */
  inline const JsonElement *first() const { return (JsonElement *)_list.first(); }

  /** Remove and return the first element in the list.  This does not call the element's cleanup handler.
   *
   * @return The first element in the list or NULL if the list has no elements.
   */
  inline JsonElement *removeFirst() { return (JsonElement *)_list.removeFirst(); }

  /** Add an element to the front of the list.
   *
   * @param The element to add to the front of the list.
   */
  inline void addFirst(JsonElement *element) { _list.addFirst(element); }

  /** Get the last element in the list, but do not remove the element from the list.
   *
   * @return The last element in the list or NULL if the list has no elements
   */
  inline JsonElement *last() { return (JsonElement *)_list.last(); }

  /** Get the last element in the list, but do not remove the element from the list.
   *
   * @return The last element in the list or NULL if the list has no elements
   */
  inline const JsonElement *last() const { return (JsonElement *)_list.last(); }

  /** Remove and return the last element in the list.  This does not call the element's cleanup handler.
   *
   * @return The last element in the list or NULL if the list has no elements
   */
  inline JsonElement *removeLast() { return (JsonElement *)_list.removeLast(); }

  /** Add an element to the end of the list
   *
   * @param element The element to add to the end of the list
   */
  inline void addLast(JsonElement *element) { _list.addLast(element); }

  /** Remove an element from an arbitrary position in the list.  This does not call the element's cleanup handler.
   *
   * @param element the element to remove
   */
  inline void remove(JsonElement *element) { _list.remove(element); }

  /** Return the element at an index where the first element is at index 0. O(n).
   *
   * @param idx the index
   * @return The element at the index or NULL if there is no element at the index.
   */
  inline JsonElement *index(int idx) { return (JsonElement *)_list.index(idx); }

 private:
  EmbeddedList _list;

  ESB_DEFAULT_FUNCS(JsonList);
};

}  // namespace ESB

#endif
