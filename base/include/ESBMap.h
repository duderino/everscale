#ifndef ESB_MAP_H
#define ESB_MAP_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_LOCKABLE_H
#include <ESBLockable.h>
#endif

#ifndef ESB_COMPARATOR_H
#include <ESBComparator.h>
#endif

#ifndef ESB_NULL_LOCK_H
#include <ESBNullLock.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ESB {

/** This class is the internal node used by Map and MapIterator.  It
 *  should not be used directly by client code.
 */
class MapNode {
 public:
  MapNode(MapNode *parent, MapNode *left, MapNode *right, bool isBlack,
          const void *key, void *value);

  ~MapNode();

  inline void *operator new(size_t size, Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

  MapNode *_parent;
  MapNode *_left;
  MapNode *_right;
  bool _isBlack;
  const void *_key;
  void *_value;

 private:
  // disabled
  MapNode(const MapNode &);
  MapNode &operator=(const MapNode &);
};

class MapIterator;

/** Map is a red-black balanced binary tree that implements Cormen,
 *  Leiserson, and Rivest's red-black tree in "Introduction to Algorithms".
 *
 *  @ingroup collection
 */
class Map : public Lockable {
  friend class MapIterator;

 public:
  /** Constructor.
   *
   *  @param comparator The comparator that will be used to maintain the
   *      tree nodes in sorted order.
   *  @param allocator The allocator that the map will use to create its
   *      internal nodes.
   *  @param lockable A lock that will be used to synchronize the map.  All
   *      of the map's synchronizing methods will forward to this object.
   *
   *  @see GetAllocationSize to determine how much memory the map will
   *      allocate for every internal node it creates.  This is useful for
   *      constructing fixed length allocators.
   */
  Map(Comparator &comparator, Lockable &lockable = NullLock::Instance(),
      Allocator &allocator = SystemAllocator::Instance());

  /** Destructor. */
  virtual ~Map();

  /** Insert a key/value pair into the map.  O(lg n).
   *
   *  @param key The key to insert.
   *  @param value The value to insert.  The value may be NULL.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   *      ESB_UNIQUENESS_VIOLATION will be returned if the key already
   *      exists and the map enforces uniqueness.
   */
  Error insert(const void *key, void *value);

  /** Erase a key/value pair from the map given its key.  O(lg n).
   *  <p>
   *  Note, this will only deallocate memory used by the map's internal nodes.
   *  </p>
   *
   *  @param key The key of the key/value pair to erase.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   *      ESB_CANNOT_FIND will be returned if the key cannot be found.
   */
  Error remove(const void *key);

  /** Find a value in the map given its key.  O(lg n).
   *
   *  @param key The key of the key/value pair to find.
   *  @return The value or NULL if the value cannot be found.
   */
  void *find(const void *key);

  /** Update key/value pair in the map given its key.  O(lg n).
   *
   *  @param key The key of the key/value pair.
   *  @param value A new value to assign to the key/value pair.  Any resources
   *      used by the old value will not be released by this method.  The
   *      value may be NULL.
   *  @param old If non-NULL, the old value will be assigned to this.
   *  @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key cannot
   *      be found, another error code otherwise.
   */
  Error update(const void *key, void *value, void **old);

  /** Remove all key/value pairs from the map.  O(n lg n).
   *  <p>
   *  This will only deallocate memory used by the map's internal nodes.
   *  </p>
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   *      Clearing an empty map will return ESB_SUCCESS.
   */
  Error clear();

  /** Get an iterator pointing to the smallest element of the map.  If the
   *  map is empty, the getKey method of the iterator will return NULL
   *  and the isNull method will return true.  O(lg n).
   *  <p>
   *  Note that the map itself must be synchronized whenever any of the
   *  iterators methods are called if the map is shared by multiple threads.
   *  </p>
   *
   *  @return An iterator pointing to the first element in the map.
   */
  MapIterator minimumIterator();

  /** Get an iterator pointing to the largest element of the map.  If the
   *  map is empty, the getKey method of the iterator will return NULL
   *  and the isNull method will return true.  O(lg n).
   *  <p>
   *  Note that the map itself must be synchronized whenever any of the
   *  iterators methods are called if the map is shared by multiple threads.
   *  </p>
   *
   *  @return An iterator pointing to the last element in the map.
   */
  MapIterator maximumIterator();

  /** Insert a key/value pair into the map and immediately get back its
   *  iterator. O(lg n).
   *  <p>
   *  Note that the map itself must be synchronized whenever any of the
   *  iterators methods are called if the map is shared by multiple threads.
   *  </p>
   *
   *  @param key The key of the key/value pair to insert.
   *  @param value The value of the key/value pair to insert.  The value may
   *      be NULL.
   *  @param iterator The iterator will be copied here on a successful insert.
   *  @return ESB_SUCCESS if successful, ESB_UNIQUENESS_VIOLATION if the
   *      key already exists and the map enforces uniqueness, another error
   *      code otherwise.
   */
  Error insert(const void *key, void *value, MapIterator *iterator);

  /** Find an iterator in the map given its key.  O(lg n).
   *  <p>
   *  Note that the map itself must be synchronized whenever any of the
   *  iterators methods are called if the map is shared by multiple threads.
   *  </p>
   *
   *  @param key The key of the key/value pair to find.
   *  @param iterator An iterator pointing to the key/value pair will be
   *      copied here if the key can be found.
   *  @return An iterator pointing to the value.  If the key cannot be found,
   *      the returned iterator's isNull() method will return true.
   */
  MapIterator findIterator(const void *key);

  /** Remove the key/value pair pointed to by this iterator from the map.
   *  O(lg n) -- but faster than a find & erase.
   *  <p>
   *  Note that the iterator will be invalidated in the process.  In fact,
   *  all iterators that point to the key/value pair pointed to by this
   *  iterator will be invalidated.  Using invalid iterators can cause memory
   *  corruption.
   *  </p>
   *  <p>
   *  Note that this will only deallocate memory used by the map's internal
   *  nodes.  It will not deallocate memory used by the key and value
   *  components of the key/value pair.
   *  </p>
   *
   *  @param iterator The iterator that points to the element to remove from
   *      the map.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   *      ESB_INVALID_ITERATOR will be returned if the iterator does not
   *      point to a key/value pair.
   */
  Error erase(MapIterator *iterator);

  /** Get the current size of the map.  O(1).
   *
   *  @return The current size of the map.
   */
  UInt32 size() const;

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

  /** Get the size of the Map's internal nodes.  All allocation calls to
   *  the memory allocator will always request this number of bytes.
   *
   *  @return The size in bytes of the Map's internal nodes.
   */
  Size AllocationSize();

  /** Determine whether the tree is balanced.  Used only by the unit tests.
   *
   *  @return true if the tree is balanced, false otherwise.
   */
  bool isBalanced() const;

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return memory for the object, or NULL if it couldn't be allocated.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 private:
  // Disabled
  Map(const Map &);
  Map &operator=(const Map &);

  static MapNode *findMinimum(MapNode *x);
  static MapNode *findMaximum(MapNode *x);
  static MapNode *findSuccessor(MapNode *x);
  static MapNode *findPredecessor(MapNode *x);
  MapNode *findNode(MapNode *x, const void *k);
  bool insertNode(MapNode *z);
  void deleteNode(MapNode *z);
  void rightRotate(MapNode *x);
  void leftRotate(MapNode *x);
  int getBlackHeight(MapNode *node, bool *unbalanced) const;
  int getHeight(MapNode *node) const;

  UInt32 _size;
  MapNode *_root;
  Allocator &_allocator;
  Lockable &_lockable;
  Comparator &_comparator;
  MapNode _sentinel;
};

/** MapIterator supports iteration through the Map class.  It's getNext
 *  method can be used for an in-order tree walk, it's getPrevious method can
 *  be used for a reverse-order tree walk.  MapIterators also follow the
 *  same iterator invalidation rules as STL Map iterators.  That is, changes
 *  to a Map instance do not invalidate any existing iterators unless that
 *  change deletes the very node that an iterator points to.  In this last
 *  case, no protection is offered to the caller.  Using an invalid
 *  iterator can cause memory corruption.
 *
 *  @ingroup collection
 */
class MapIterator {
  friend class Map;

 public:
  /** Default Constructor. */
  MapIterator();

  /** Copy constructor.
   *
   *  @param iterator the iterator to copy.
   */
  MapIterator(const MapIterator &iterator);

  /** Destructor. */
  ~MapIterator();

  /** Assignment operator.
   *
   *  @param iterator the iterator to copy.
   */
  inline MapIterator &operator=(const MapIterator &iterator) {
    _node = iterator._node;
    return *this;
  }

  /** Determine whether there is another key/value pair after the
   *  key/value pair pointed to by this iterator. O(lg n).
   *
   *  @return true if there is, false otherwise.
   */
  inline bool hasNext() {
    return _node ? (NULL != Map::findSuccessor(_node)->_key) : false;
  }

  /** Pre-increment operator.  O(lg n).  Point this iterator at the next
   *  key/value pair in the map.  If there is no next key/value pair, the
   *  iterator will be set to null (i.e., its isNull method will return
   *  true and its getKey method will return NULL).
   *
   *  @return The iterator itself
   */
  inline MapIterator &operator++() {
    _node = _node ? Map::findSuccessor(_node) : 0;
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
    MapIterator it(_node);
    _node = _node ? Map::findSuccessor(_node) : 0;
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
    MapIterator node(_node ? Map::findSuccessor(_node) : 0);
    return node;
  }

  /** Pre-decrement operator.  O(lg n).  Point this iterator at the previous
   *  key/value pair in the map.  If there is no previous key/value pair, the
   *  iterator will be set to null (i.e., its isNull method will return
   *  true and its getKey method will return NULL).
   *
   *  @return The iterator itself
   */
  inline MapIterator &operator--() {
    _node = _node ? Map::findPredecessor(_node) : 0;
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
    MapIterator it(_node);
    _node = _node ? Map::findPredecessor(_node) : 0;
    return it;
  }

  /** Determine whether there is another key/value pair before the
   *  key/value pair pointed to by this iterator. O(lg n).
   *
   *  @return true if there is, false otherwise.
   */
  inline bool hasPrevious() {
    return _node ? (NULL != Map::findPredecessor(_node)->_key) : false;
  }

  /** Get an iterator for the key/value pair before the key/value pair
   *  pointed to by this iterator.  O(lg n).  If there is no previous
   *  key/value pair, a null iterator will be returned (i.e., its isNull
   *  method will return true and its getKey method will return NULL).
   *
   *  @return The previous iterator.
   */
  inline MapIterator previous() {
    MapIterator node(_node ? Map::findPredecessor(_node) : 0);
    return node;
  }

  /** Get the key of the key/value pair that this iterator points to.  O(1).
   *
   *  @return The key or NULL if the iterator does not point to a key/value
   *      pair.
   */
  inline const void *key() { return _node ? _node->_key : 0; }

  /** Get the value of the key/value pair that this iterator points to.  O(1).
   *
   *  @return The value or NULL if the iterator does not point to a key/value
   *      pair.
   */
  inline void *value() { return _node ? _node->_value : 0; }

  /** Set the value of the key/value pair that this iterator points to.  O(1).
   *  <p>
   *  This operation does not free any memory allocated to the old value.
   *  </p>
   *
   *  @param value The new value of the element.
   */
  inline void setValue(void *value) {
    if (!_node || !_node->_key) return;

    _node->_value = value;
  }

  /** Determine whether the iterator is null.  Null iterators are iterators
   *  that do not point to a key/value pair.  Iterators are typically null
   *  when they are first created and when used to iterate past the end or
   *  before the beginning of the map.
   *
   *  @return true if the iterator is null, false otherwise.
   */
  inline bool isNull() { return !_node || !_node->_key; }

  /** Compare two iterators for equality.  Two iterators are equal if they
   *  point to the same key/value pair.  In maps that do not enforce
   *  unique keys, iterators must point to the same internal node.  That is,
   *  two iterators can compare unequal even if their keys and values are
   *  equal.
   *
   *  @return true if both iterators point to the same key/value pair or if
   *      both iterators are null (do not point to any key/value pair).
   */
  inline bool operator==(const MapIterator &iterator) {
    return _node == iterator._node;
  }

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
  /** Constructor.
   *
   *  @param node The key/value pair that this iterator initially points to.
   */
  MapIterator(MapNode *node);

  MapNode *_node;
};

}  // namespace ESB

#endif
