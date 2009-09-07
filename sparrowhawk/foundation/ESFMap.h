/** @file ESFMap.h
 *  @brief A balanced binary tree.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_MAP_H
#define ESF_MAP_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_LOCKABLE_H
#include <ESFLockable.h>
#endif

#ifndef ESF_COMPARATOR_H
#include <ESFComparator.h>
#endif

/** This class is the internal node used by ESFMap and ESFMapIterator.  It
 *  should not be used directly by client code.
 */
class ESFMapNode {
public:

    ESFMapNode(ESFMapNode *parent, ESFMapNode *left, ESFMapNode *right, bool isBlack, const void *key, void *value);

    ~ESFMapNode();

    void *operator new(size_t size, ESFAllocator *allocator);

    ESFMapNode *_parent;
    ESFMapNode *_left;
    ESFMapNode *_right;
    bool _isBlack;
    const void *_key;
    void *_value;

private:

    // disabled
    ESFMapNode(const ESFMapNode &);
    ESFMapNode &operator=(const ESFMapNode &);

};

class ESFMapIterator;

/** ESFMap is a red-black balanced binary tree that implements Cormen,
 *  Leiserson, and Rivest's red-black tree in "Introduction to Algorithms".
 *  Changes were made to optionally support "MultiMaps" (Maps which allow
 *  multiple elements with the same key) and iterators with the same iterator
 *  invalidation rules as the STL Map (unless you delete the node an iterator
 *  is pointing to, operations which modify the tree do not invalidate any
 *  existing iterator).
 *  <p>
 *  The keys inserted into ESFMap instances are not allowed to be NULL, though
 *  the values can be.  Up to 4,294,967,295 elements can be inserted into a
 *  ESFMap instance assuming it can always allocate enough memory for its
 *  internal nodes.
 *  </p>
 *
 *  @ingroup collection
 */
class ESFMap: public ESFLockable {
    friend class ESFMapIterator;

public:

    /** Constructor.
     *
     *  @param isUnique True if the map should enforce uniqueness, false
     *      otherwise.
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
    ESFMap(bool isUnique, ESFComparator *comparator, ESFAllocator *allocator, ESFLockable *lockable);

    /** Destructor. */
    virtual ~ESFMap();

    /** Insert a key/value pair into the map.  O(lg n).
     *
     *  @param key The key to insert.
     *  @param value The value to insert.  The value may be NULL.
     *  @return ESF_SUCCESS if successful, another error code otherwise.
     *      ESF_UNIQUENESS_VIOLATION will be returned if the key already
     *      exists and the map enforces uniqueness.
     */
    ESFError insert(const void *key, void *value);

    /** Erase a key/value pair from the map given its key.  O(lg n).
     *  <p>
     *  Note, this will only deallocate memory used by the map's internal nodes.
     *  </p>
     *
     *  @param key The key of the key/value pair to erase.
     *  @return ESF_SUCCESS if successful, another error code otherwise.
     *      ESF_CANNOT_FIND will be returned if the key cannot be found.
     */
    ESFError erase(const void *key);

    /** Find a value in the map given its key.  O(lg n).
     *
     *  @param key The key of the key/value pair to find.
     *  @param value The value of the key/value pair will be copied here if
     *      the key can be found.
     *  @return ESF_SUCCESS if successful, ESF_CANNOT_FIND if the key cannot
     *      be found, another error code otherwise.
     */
    ESFError find(const void *key, void **value);

    /** Update key/value pair in the map given its key.  O(lg n).
     *
     *  @param key The key of the key/value pair.
     *  @param value A new value to assign to the key/value pair.  Any resources
     *      used by the old value will not be released by this method.  The
     *      value may be NULL.
     *  @param old If non-NULL, the old value will be assigned to this.
     *  @return ESF_SUCCESS if successful, ESF_CANNOT_FIND if the key cannot
     *      be found, another error code otherwise.
     */
    ESFError update(const void *key, void *value, void **old);

    /** Remove all key/value pairs from the map.  O(n lg n).
     *  <p>
     *  This will only deallocate memory used by the map's internal nodes.
     *  </p>
     *
     *  @return ESF_SUCCESS if successful, another error code otherwise.
     *      Clearing an empty map will return ESF_SUCCESS.
     */
    ESFError clear();

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
    ESFMapIterator getMinimumIterator();

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
    ESFMapIterator getMaximumIterator();

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
     *  @return ESF_SUCCESS if successful, ESF_UNIQUENESS_VIOLATION if the
     *      key already exists and the map enforces uniqueness, another error
     *      code otherwise.
     */
    ESFError insert(const void *key, void *value, ESFMapIterator *iterator);

    /** Find an iterator in the map given its key.  O(lg n).
     *  <p>
     *  Note that the map itself must be synchronized whenever any of the
     *  iterators methods are called if the map is shared by multiple threads.
     *  </p>
     *
     *  @param key The key of the key/value pair to find.
     *  @param iterator An iterator pointing to the key/value pair will be
     *      copied here if the key can be found.
     *  @return ESF_SUCCESS if successful, ESF_CANNOT_FIND if the key cannot be
     *      found, another error code otherwise.
     */
    ESFError find(const void *key, ESFMapIterator *iterator);

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
     *  @return ESF_SUCCESS if successful, another error code otherwise.
     *      ESF_INVALID_ITERATOR will be returned if the iterator does not
     *      point to a key/value pair.
     */
    ESFError erase(ESFMapIterator *iterator);

    /** Get the current size of the map.  O(1).
     *
     *  @return The current size of the map.
     */
    ESFUInt32 getSize() const;

    /** Determine whether the map is empty.  O(1).
     *
     *  @return true if the map is empty, false otherwise.
     */
    bool isEmpty() const;

    /** Block the calling thread until write access is granted.
     *
     *  @return ESF_SUCCESS if successful, another error code otherwise.
     */
    virtual ESFError writeAcquire();

    /** Block the calling thread until read access is granted.
     *
     *  @return ESF_SUCCESS if successful, another error code otherwise.
     */
    virtual ESFError readAcquire();

    /** Attempt to gain write access, returning immediately if access could not
     *  be granted.
     *
     *  @return ESF_SUCCESS if access was granted, ESF_AGAIN if access could
     *      not be immediately granted, or another error code if an error
     *      occurred.
     */
    virtual ESFError writeAttempt();

    /** Attempt to gain read access, returning immediately if access could not
     *  be granted.
     *
     *  @return ESF_SUCCESS if access was granted, ESF_AGAIN if access could
     *      not be immediately granted, or another error code if an error
     *      occurred.
     */
    virtual ESFError readAttempt();

    /** Release the lock after write access was granted.
     *
     *  @return ESF_SUCCESS if successful, another error code otherwise.
     */
    virtual ESFError writeRelease();

    /** Release the lock after read access was granted.
     *
     *  @return ESF_SUCCESS if successful, another error code otherwise.
     */
    virtual ESFError readRelease();

    /** Get the size of the Map's internal nodes.  All allocation calls to
     *  the memory allocator will always request this number of bytes.
     *
     *  @return The size in bytes of the Map's internal nodes.
     */
    ESFSize GetAllocationSize();

#ifdef DEBUG
    /** Determine whether the tree is balanced.  Used only by the unit tests.
     *
     *  @return true if the tree is balanced, false otherwise.
     */
    bool isBalanced() const;
#endif

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return memory for the object, or NULL if it couldn't be allocated.
     */
    inline void *operator new(size_t size, ESFAllocator *allocator) {
        return allocator->allocate(size);
    }

private:

    // Disabled
    ESFMap(const ESFMap &);
    ESFMap &operator=(const ESFMap &);

    static ESFMapNode *findMinimum(ESFMapNode *x);
    static ESFMapNode *findMaximum(ESFMapNode *x);
    static ESFMapNode *findSuccessor(ESFMapNode *x);
    static ESFMapNode *findPredecessor(ESFMapNode *x);

    ESFMapNode *findNode(ESFMapNode *x, const void *k);
    bool insertNode(ESFMapNode *z);
    void deleteNode(ESFMapNode *z);
    void rightRotate(ESFMapNode *x);
    void leftRotate(ESFMapNode *x);

#ifdef DEBUG
    int getBlackHeight(ESFMapNode *node, bool *unbalanced) const;
    int getHeight(ESFMapNode *node) const;
#endif

    ESFUInt32 _size;
    bool _isUnique;

    ESFMapNode *_root;
    ESFAllocator *_allocator;
    ESFLockable *_lockable;
    ESFComparator *_comparator;

    ESFMapNode _sentinel;
};

/** ESFMapIterator supports iteration through the ESFMap class.  It's getNext
 *  method can be used for an in-order tree walk, it's getPrevious method can
 *  be used for a reverse-order tree walk.  ESFMapIterators also follow the
 *  same iterator invalidation rules as STL Map iterators.  That is, changes
 *  to a ESFMap instance do not invalidate any existing iterators unless that
 *  change deletes the very node that an iterator points to.  In this last
 *  case, no protection is offered to the caller.  Using an invalid
 *  iterator can cause memory corruption.
 *
 *  @ingroup collection
 */
class ESFMapIterator {
    friend class ESFMap;

public:

    /** Default Constructor. */
    ESFMapIterator();

    /** Copy constructor.
     *
     *  @param iterator the iterator to copy.
     */
    ESFMapIterator(const ESFMapIterator &iterator);

    /** Destructor. */
    ~ESFMapIterator();

    /** Assignment operator.
     *
     *  @param iterator the iterator to copy.
     */
    inline ESFMapIterator &operator=(const ESFMapIterator &iterator) {
        _node = iterator._node;

        return *this;
    }

    /** Determine whether there is another key/value pair after the
     *  key/value pair pointed to by this iterator. O(lg n).
     *
     *  @return true if there is, false otherwise.
     */
    inline bool hasNext() {
        return _node ? (!ESFMap::findSuccessor(_node)->_key) : false;
    }

    /** Pre-increment operator.  O(lg n).  Point this iterator at the next
     *  key/value pair in the map.  If there is no next key/value pair, the
     *  iterator will be set to null (i.e., its isNull method will return
     *  true and its getKey method will return NULL).
     *
     *  @return The iterator itself
     */
    inline ESFMapIterator &operator++() {
        _node = _node ? ESFMap::findSuccessor(_node) : 0;

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
    inline ESFMapIterator operator++(int) {
        ESFMapIterator it(_node);

        _node = _node ? ESFMap::findSuccessor(_node) : 0;

        return it;
    }

    /** Get an iterator for the key/value pair after the key/value pair
     *  pointed to by this iterator.  O(lg n).  If there is no next
     *  key/value pair, a null iterator will be returned (i.e., its isNull
     *  method will return true and its getKey method will return NULL).
     *
     *  @return The next iterator.
     */
    inline ESFMapIterator getNext() {
        ESFMapIterator node(_node ? ESFMap::findSuccessor(_node) : 0);

        return node;
    }

    /** Pre-decrement operator.  O(lg n).  Point this iterator at the previous
     *  key/value pair in the map.  If there is no previous key/value pair, the
     *  iterator will be set to null (i.e., its isNull method will return
     *  true and its getKey method will return NULL).
     *
     *  @return The iterator itself
     */
    inline ESFMapIterator &operator--() {
        _node = _node ? ESFMap::findPredecessor(_node) : 0;

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
    inline ESFMapIterator operator--(int) {
        ESFMapIterator it(_node);

        _node = _node ? ESFMap::findPredecessor(_node) : 0;

        return it;
    }

    /** Determine whether there is another key/value pair before the
     *  key/value pair pointed to by this iterator. O(lg n).
     *
     *  @return true if there is, false otherwise.
     */
    inline bool hasPrevious() {
        return _node ? (!ESFMap::findPredecessor(_node)->_key) : false;
    }

    /** Get an iterator for the key/value pair before the key/value pair
     *  pointed to by this iterator.  O(lg n).  If there is no previous
     *  key/value pair, a null iterator will be returned (i.e., its isNull
     *  method will return true and its getKey method will return NULL).
     *
     *  @return The previous iterator.
     */
    inline ESFMapIterator getPrevious() {
        ESFMapIterator node(_node ? ESFMap::findPredecessor(_node) : 0);

        return node;
    }

    /** Get the key of the key/value pair that this iterator points to.  O(1).
     *
     *  @return The key or NULL if the iterator does not point to a key/value
     *      pair.
     */
    inline const void *getKey() {
        return _node ? _node->_key : 0;
    }

    /** Get the value of the key/value pair that this iterator points to.  O(1).
     *
     *  @return The value or NULL if the iterator does not point to a key/value
     *      pair.
     */
    inline void *getValue() {
        return _node ? _node->_value : 0;
    }

    /** Set the value of the key/value pair that this iterator points to.  O(1).
     *  <p>
     *  This operation does not free any memory allocated to the old value.
     *  </p>
     *
     *  @param value The new value of the element.
     */
    inline void setValue(void *value) {
        if (!_node || !_node->_key)
            return;

        _node->_value = value;
    }

    /** Determine whether the iterator is null.  Null iterators are iterators
     *  that do not point to a key/value pair.  Iterators are typically null
     *  when they are first created and when used to iterate past the end or
     *  before the beginning of the map.
     *
     *  @return true if the iterator is null, false otherwise.
     */
    inline bool isNull() {
        return !_node || !_node->_key;
    }

    /** Compare two iterators for equality.  Two iterators are equal if they
     *  point to the same key/value pair.  In maps that do not enforce
     *  unique keys, iterators must point to the same internal node.  That is,
     *  two iterators can compare unequal even if their keys and values are
     *  equal.
     *
     *  @return true if both iterators point to the same key/value pair or if
     *      both iterators are null (do not point to any key/value pair).
     */
    inline bool operator==(const ESFMapIterator &iterator) {
        return _node == iterator._node;
    }

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return The new object or NULL of the memory allocation failed.
     */
    inline void *operator new(size_t size, ESFAllocator *allocator) {
        return allocator->allocate(size);
    }

private:

    /** Constructor.
     *
     *  @param node The key/value pair that this iterator initially points to.
     */
    ESFMapIterator(ESFMapNode *node);

    ESFMapNode *_node;
};

#endif /* ! ESF_MAP_H */
