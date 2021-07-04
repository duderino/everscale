#ifndef ESB_JSON_MAP_H
#define ESB_JSON_MAP_H

#ifndef ESB_JSON_SCALAR_H
#include <ESBJsonScalar.h>
#endif

#ifndef ESB_MAP_H
#include <ESBMap.h>
#endif

namespace ESB {

class JsonMapIterator;

/** A JSON Map.
 *
 *  @ingroup json
 */
class JsonMap : public JsonElement {
 public:
  /** Constructor.
   */
  JsonMap(Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~JsonMap();

  virtual Type type() const;

  /** Insert a key/value pair into the map.  O(lg n).
   *
   *  @param key The key to insert.
   *  @param value The value to insert.  The value may be NULL.
   *  @return ESB_SUCCESS if successful, another error code otherwise.  ESB_UNIQUENESS_VIOLATION will be returned if the
   * key already exists
   */
  inline Error insert(const JsonScalar *key, JsonElement *value) { return _map.insert(key, value); }

  /** Erase a key/value pair from the map given its key.  O(lg n).
   *  <p>
   *  Note, this will only deallocate memory used by the map's internal nodes.
   *  </p>
   *
   *  @param key The key of the key/value pair to erase.
   *  @return ESB_SUCCESS if successful, another error code otherwise.  ESB_CANNOT_FIND will be returned if the key
   * cannot be found.
   */
  inline Error remove(const JsonScalar *key) { return _map.remove(key); }

  /** Find a value in the map given its key.  O(lg n).
   *
   *  @param key The key of the key/value pair to find.
   *  @return The value or NULL if the value cannot be found.
   */
  inline JsonElement *find(const JsonScalar *key) { return (JsonElement *)_map.find(key); }

  /** Update key/value pair in the map given its key.  O(lg n).
   *
   *  @param key The key of the key/value pair.
   *  @param value A new value to assign to the key/value pair.  Any resources used by the old value will not be
   * released by this method.  The value may be NULL.
   *  @param old If non-NULL, the old value will be assigned to this.
   *  @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key cannot be found, another error code otherwise.
   */
  inline Error update(const JsonScalar *key, JsonElement *value, JsonElement **old) {
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
  JsonMapIterator iterator();

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
  Map _map;

  ESB_DEFAULT_FUNCS(JsonMap);
};

class JsonMapIterator {
  friend class JsonMap;  // So JsomMap can call the private ctor

 public:
  /** Default Constructor. */
  JsonMapIterator();

  /** Copy constructor.
   *
   *  @param iterator the iterator to copy.
   */
  JsonMapIterator(const JsonMapIterator &it);

  /** Destructor. */
  ~JsonMapIterator();

  /** Assignment operator.
   *
   *  @param iterator the iterator to copy.
   */
  inline JsonMapIterator &operator=(const JsonMapIterator &it) {
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
  inline JsonMapIterator &operator++() {
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
  inline JsonMapIterator operator++(int) {
    JsonMapIterator it(_it);
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
  inline JsonMapIterator next() {
    JsonMapIterator it(_it.next());
    return it;
  }

  /** Pre-decrement operator.  O(lg n).  Point this iterator at the previous
   *  key/value pair in the map.  If there is no previous key/value pair, the
   *  iterator will be set to null (i.e., its isNull method will return
   *  true and its getKey method will return NULL).
   *
   *  @return The iterator itself
   */
  inline JsonMapIterator &operator--() {
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
  inline JsonMapIterator operator--(int) {
    JsonMapIterator it(_it);
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
  inline JsonMapIterator previous() {
    JsonMapIterator it(_it.previous());
    return it;
  }

  /** Get the key of the key/value pair that this iterator points to.  O(1).
   *
   *  @return The key or NULL if the iterator does not point to a key/value
   *      pair.
   */
  inline const JsonScalar *key() { return (JsonScalar *)_it.key(); }

  /** Get the value of the key/value pair that this iterator points to.  O(1).
   *
   *  @return The value or NULL if the iterator does not point to a key/value
   *      pair.
   */
  inline JsonElement *value() { return (JsonElement *)_it.value(); }

  /** Set the value of the key/value pair that this iterator points to.  O(1).
   *  <p>
   *  This operation does not free any memory allocated to the old value.
   *  </p>
   *
   *  @param value The new value of the element.
   */
  inline void setValue(JsonElement *value) { _it.setValue(value); }

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
  inline bool operator==(const JsonMapIterator &it) { return _it == it._it; }

 private:
  /** Constructor.
   *
   *  @param node The key/value pair that this iterator initially points to.
   */
  JsonMapIterator(const MapIterator &it);

  MapIterator _it;

  ESB_PLACEMENT_NEW(JsonMapIterator);
};

}  // namespace ESB

#endif
