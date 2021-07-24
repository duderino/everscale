#ifndef ESB_AST_MAP_H
#define ESB_AST_MAP_H

#ifndef ESB_AST_SCALAR_H
#include <ASTScalar.h>
#endif

#ifndef ESB_MAP_H
#include <ESBMap.h>
#endif

#ifndef ESB_AST_STRING_H
#include <ASTString.h>
#endif

namespace ESB {
namespace AST {

class MapIterator;

/** An AST Map.
 *
 *  @ingroup ast
 */
class Map : public Element {
 public:
  /** Constructor.
   */
  Map(Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~Map();

  virtual Type type() const;

  /** Insert a key/value pair into the map.  O(lg n).
   *
   *  @param key The key to insert.
   *  @param value The value to insert.  The value may be NULL.
   *  @return ESB_SUCCESS if successful, another error code otherwise.  ESB_UNIQUENESS_VIOLATION will be returned if the
   * key already exists
   */
  inline Error insert(const Scalar *key, Element *value) { return _map.insert(key, value); }

  /** Erase a key/value pair from the map given its key.  O(lg n).
   *  <p>
   *  Note, this will only deallocate memory used by the map's internal nodes.
   *  </p>
   *
   *  @param key The key of the key/value pair to erase.
   *  @return ESB_SUCCESS if successful, another error code otherwise.  ESB_CANNOT_FIND will be returned if the key
   * cannot be found.
   */
  inline Error remove(const Scalar *key) { return _map.remove(key); }

  /** Find a value in the map given its key.  O(lg n).
   *
   *  @param key The key of the key/value pair to find.
   *  @return The value or NULL if the value cannot be found.
   */
  inline Element *find(const Scalar *key) { return (Element *)_map.find(key); }

  /** Find a value in the map given its key.  O(lg n).
   *
   *  @param key The key of the key/value pair to find.
   *  @return The value or NULL if the value cannot be found.
   */
  inline const Element *find(const Scalar *key) const { return (const Element *)_map.find(key); }

  /** Find a value in the map given its key.  O(lg n).
   *
   *  @param key The key of the key/value pair to find.
   *  @return The value or NULL if the value cannot be found.
   */
  Element *find(const char *key);

  /** Find a value in the map given its key.  O(lg n).
   *
   *  @param key The key of the key/value pair to find.
   *  @return The value or NULL if the value cannot be found.
   */
  const Element *find(const char *key) const;

  /** Update key/value pair in the map given its key.  O(lg n).
   *
   *  @param key The key of the key/value pair.
   *  @param value A new value to assign to the key/value pair.  Any resources used by the old value will not be
   * released by this method.  The value may be NULL.
   *  @param old If non-NULL, the old value will be assigned to this.
   *  @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key cannot be found, another error code otherwise.
   */
  inline Error update(const Scalar *key, Element *value, Element **old) {
    return _map.update(key, value, (void **)old);
  }

  /** Remove all key/value pairs from the map.  O(n lg n).
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.  Clearing an empty map will return ESB_SUCCESS.
   */
  Error clear();

  /** Get an iterator pointing to the first element of the map.  If the
   *  map is empty, the getKey method of the iterator will return NULL
   *  and the isNull method will return true.  O(lg n).
   *
   *  @return An iterator pointing to the first element in the map.
   */
  MapIterator iterator();

  /** Get the current size of the map.  O(1).
   *
   *  @return The current size of the map.
   */
  inline UInt32 size() const { return _map.size(); }

 private:
  class JsonMapComparator : public Comparator {
   public:
    virtual int compare(const void *left, const void *right) const;
  };

  static JsonMapComparator _Comparator;
  ESB::Map _map;

  ESB_DEFAULT_FUNCS(Map);
};

class MapIterator {
  friend class Map;  // So JsomMap can call the private ctor

 public:
  /** Default Constructor. */
  MapIterator();

  /** Copy constructor.
   *
   *  @param iterator the iterator to copy.
   */
  MapIterator(const MapIterator &it);

  /** Destructor. */
  ~MapIterator();

  /** Assignment operator.
   *
   *  @param iterator the iterator to copy.
   */
  inline MapIterator &operator=(const MapIterator &it) {
    _it = it._it;
    return *this;
  }

  /** Determine whether there is another key/value pair after the
   *  key/value pair pointed to by this iterator. O(lg n).
   *
   *  @return true if there is, false otherwise.
   */
  inline bool hasNext() { return _it.hasNext(); }

  /** Pre-increment operator.  O(lg n).  Point this iterator at the next
   *  key/value pair in the map.  If there is no next key/value pair, the
   *  iterator will be set to null (i.e., its isNull method will return
   *  true and its getKey method will return NULL).
   *
   *  @return The iterator itself
   */
  inline MapIterator &operator++() {
    _it++;
    return *this;
  }

  /** Post-increment operator.  O(lg n).  Point this iterator at the next
   *  key/value pair in the map.  If there is no next key/value pair, the
   *  iterator will be set to null (i.e., its isNull method will return
   *  true and its getKey method will return NULL).
   *
   *  @return A new iterator pointing to the key/value pair before the
   *      increment operation.
   */
  inline MapIterator operator++(int) {
    MapIterator it(_it);
    _it++;
    return it;
  }

  /** Get an iterator for the key/value pair after the key/value pair
   *  pointed to by this iterator.  O(lg n).  If there is no next
   *  key/value pair, a null iterator will be returned (i.e., its isNull
   *  method will return true and its getKey method will return NULL).
   *
   *  @return The next iterator.
   */
  inline MapIterator next() {
    MapIterator it(_it.next());
    return it;
  }

  /** Pre-decrement operator.  O(lg n).  Point this iterator at the previous
   *  key/value pair in the map.  If there is no previous key/value pair, the
   *  iterator will be set to null (i.e., its isNull method will return
   *  true and its getKey method will return NULL).
   *
   *  @return The iterator itself
   */
  inline MapIterator &operator--() {
    _it--;
    return *this;
  }

  /** Post-decrement operator.  O(lg n).  Point this iterator at the previous
   *  key/value pair in the map.  If there is no previous key/value pair, the
   *  iterator will be set to null (i.e., its isNull method will return
   *  true and its getKey method will return NULL).
   *
   *  @return A new iterator pointing to the key/value pair before the
   *      increment operation.
   */
  inline MapIterator operator--(int) {
    MapIterator it(_it);
    _it--;
    return it;
  }

  /** Determine whether there is another key/value pair before the
   *  key/value pair pointed to by this iterator. O(lg n).
   *
   *  @return true if there is, false otherwise.
   */
  inline bool hasPrevious() { return _it.hasPrevious(); }

  /** Get an iterator for the key/value pair before the key/value pair
   *  pointed to by this iterator.  O(lg n).  If there is no previous
   *  key/value pair, a null iterator will be returned (i.e., its isNull
   *  method will return true and its getKey method will return NULL).
   *
   *  @return The previous iterator.
   */
  inline MapIterator previous() {
    MapIterator it(_it.previous());
    return it;
  }

  /** Get the key of the key/value pair that this iterator points to.  O(1).
   *
   *  @return The key or NULL if the iterator does not point to a key/value
   *      pair.
   */
  inline const Scalar *key() { return (Scalar *)_it.key(); }

  /** Get the value of the key/value pair that this iterator points to.  O(1).
   *
   *  @return The value or NULL if the iterator does not point to a key/value
   *      pair.
   */
  inline Element *value() { return (Element *)_it.value(); }

  /** Set the value of the key/value pair that this iterator points to.  O(1).
   *  <p>
   *  This operation does not free any memory allocated to the old value.
   *  </p>
   *
   *  @param value The new value of the element.
   */
  inline void setValue(Element *value) { _it.setValue(value); }

  /** Determine whether the iterator is null.  Null iterators are iterators
   *  that do not point to a key/value pair.  Iterators are typically null
   *  when they are first created and when used to iterate past the end or
   *  before the beginning of the map.
   *
   *  @return true if the iterator is null, false otherwise.
   */
  inline bool isNull() { return _it.isNull(); }

  /** Compare two iterators for equality.  Two iterators are equal if they
   *  point to the same key/value pair.  In maps that do not enforce
   *  unique keys, iterators must point to the same internal node.  That is,
   *  two iterators can compare unequal even if their keys and values are
   *  equal.
   *
   *  @return true if both iterators point to the same key/value pair or if
   *      both iterators are null (do not point to any key/value pair).
   */
  inline bool operator==(const MapIterator &it) { return _it == it._it; }

 private:
  /** Constructor.
   *
   *  @param node The key/value pair that this iterator initially points to.
   */
  MapIterator(const ESB::MapIterator &it);

  ESB::MapIterator _it;

  ESB_PLACEMENT_NEW(MapIterator);
};

}  // namespace AST
}  // namespace ESB

#endif
