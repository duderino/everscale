#ifndef ESB_AST_LIST_H
#define ESB_AST_LIST_H

#ifndef ESB_AST_ELEMENT_H
#include <ASTElement.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

namespace ESB {
namespace AST {

/** A list of AST elements
 *
 *  @ingroup ast
 */
class List : public Element {
 public:
  /** Constructor.
   */
  List(Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~List();

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
  inline Element *first() { return (Element *)_list.first(); }

  /** Get the first element in the list, but do not remove the element from the list.
   *
   * @return The first element in the list or NULL if the list has no elements
   */
  inline const Element *first() const { return (Element *)_list.first(); }

  /** Remove and return the first element in the list.  This does not call the element's cleanup handler.
   *
   * @return The first element in the list or NULL if the list has no elements.
   */
  inline Element *removeFirst() { return (Element *)_list.removeFirst(); }

  /** Add an element to the front of the list.
   *
   * @param The element to add to the front of the list.
   */
  inline void addFirst(Element *element) { _list.addFirst(element); }

  /** Get the last element in the list, but do not remove the element from the list.
   *
   * @return The last element in the list or NULL if the list has no elements
   */
  inline Element *last() { return (Element *)_list.last(); }

  /** Get the last element in the list, but do not remove the element from the list.
   *
   * @return The last element in the list or NULL if the list has no elements
   */
  inline const Element *last() const { return (Element *)_list.last(); }

  /** Remove and return the last element in the list.  This does not call the element's cleanup handler.
   *
   * @return The last element in the list or NULL if the list has no elements
   */
  inline Element *removeLast() { return (Element *)_list.removeLast(); }

  /** Add an element to the end of the list
   *
   * @param element The element to add to the end of the list
   */
  inline void addLast(Element *element) { _list.addLast(element); }

  /** Remove an element from an arbitrary position in the list.  This does not call the element's cleanup handler.
   *
   * @param element the element to remove
   */
  inline void remove(Element *element) { _list.remove(element); }

  /** Return the element at an index where the first element is at index 0. O(n).
   *
   * @param idx the index
   * @return The element at the index or NULL if there is no element at the index.
   */
  inline Element *index(int idx) { return (Element *)_list.index(idx); }

 private:
  EmbeddedList _list;

  ESB_DEFAULT_FUNCS(List);
};

}  // namespace AST
}  // namespace ESB

#endif
