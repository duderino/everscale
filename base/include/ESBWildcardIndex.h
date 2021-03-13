#ifndef ESB_WILDCARD_INDEX_H
#define ESB_WILDCARD_INDEX_H

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_EMBEDDED_MAP_ELEMENT_H
#include <ESBEmbeddedMapElement.h>
#endif

#ifndef ESB_SMART_POINTER_H
#include <ESBSmartPointer.h>
#endif

#ifndef ESB_STRING_H
#include <ESBString.h>
#endif

#ifndef ESB_SHARED_EMBEDDED_MAP_H
#include <ESBSharedEmbeddedMap.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_READ_WRITE_LOCK_H
#include <ESBReadWriteLock.h>
#endif

namespace ESB {

/**
 * WildcardIndexNodes are internal details of WildcardIndex and you should probably use WildcardIndex instead.
 * WildcardIndexNodes associate a bunch of wildcard patterns in the leftmost component of a fqdn with the remaining
 * components of the fqdn (e.g., the wildcard node can associate "foo", "f*", "*", "*o", "f*o", etc with "bar.com" and
 * can use any of these to match against "foo.bar.com").
 */
class WildcardIndexNode : public EmbeddedMapElement {
 public:
  static WildcardIndexNode *Create(const char *key, Allocator &allocator = SystemAllocator::Instance());
  static WildcardIndexNode *Recycle(const char *key, EmbeddedListElement *element);

  virtual ~WildcardIndexNode();

  virtual CleanupHandler *cleanupHandler();

  virtual const void *key() const;

  //
  // CRUD
  //

  /**
   * Insert a string key + pointer pair into the map.  O(n).
   *
   * @param key The NULL-terminated string key
   * @param value The pointer to associate with the string key
   * @param updateIfExists If true, and the string key already exists, update the value instead of failing the
   * operation.
   * @return ESB_SUCCESS if successful, ESB_UNIQUENESS_VIOLATION if the key already exists and updateIfExists is false,
   * another error code otherwise.
   */
  inline Error insert(const char *key, SmartPointer &value, bool updateIfExists = false) {
    return key ? insert(key, strlen(key), value, updateIfExists) : ESB_NULL_POINTER;
  }

  /**
   * Insert a string key + pointer pair into the map.  O(n)
   *
   * @param key The string key
   * @param keySize The size (length) of the string key
   * @param value The pointer to associate with the string key
   * @param updateIfExists If true, and the string key already exists, update the value instead of failing the
   * operation.
   * @return ESB_SUCCESS if successful, ESB_UNIQUENESS_VIOLATION if the key already exists and updateIfExists is false,
   * another error code otherwise.
   */
  Error insert(const char *key, UInt32 keySize, SmartPointer &value, bool updateIfExists = false);

  /**
   * Remove a string key from the map.  O(n)
   *
   * @param key The NULL-terminated string key to remove
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key cannot be found, another error code otherwise.
   */
  inline Error remove(const char *key) { return key ? remove(key, strlen(key)) : ESB_NULL_POINTER; }

  /**
   * Remove a string key from the map.  O(n)
   *
   * @param key The string key to remove
   * @param keySize The size (length) of the string key
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key cannot be found, another error code otherwise.
   */
  Error remove(const char *key, UInt32 keySize);

  /**
   * Return the value associated with a string key, if it can be found.  O(n)
   *
   * @param key The NULL-terminated string key to look for
   * @param value Will point to the value if found
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key cannot be found, another error code otherwise.
   */
  inline Error find(const char *key, SmartPointer &value) const {
    return key ? find(key, strlen(key), value) : ESB_NULL_POINTER;
  }

  /**
   * Return the value associated with a string key, if it can be found.  O(n)
   *
   * @param key The string key to look for
   * @param keySize The size (length) of the string key
   * @param value Will point to the value if found
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key cannot be found, another error code otherwise.
   */
  Error find(const char *key, UInt32 keySize, SmartPointer &value) const;

  /**
   * Update a string key with a new pointer.  O(n)
   *
   * @param key The NULL-terminated string key to update
   * @param keySize The size (length) of the string key
   * @param value The pointer to associate with the string key
   * @param old If not NULL, set to the key's associated value before the update.
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key doesn't exist, another error code otherwise.
   */
  inline Error update(const char *key, SmartPointer &value, SmartPointer *old) {
    return key ? update(key, strlen(key), value, old) : ESB_NULL_POINTER;
  }

  /**
   * Update a string key with a new pointer.  O(n)
   *
   * @param key The string key to update
   * @param keySize The size (length) of the string key
   * @param value The pointer to associate with the string key
   * @param old If not NULL, set to the key's associated value before the update.
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key doesn't exist, another error code otherwise.
   */
  Error update(const char *key, UInt32 keySize, SmartPointer &value, SmartPointer *old);

  /** Remove all key/value pairs from the map.  O(1).
   */
  void clear();

  //
  // Iteration. O(n). The node must not be modified (keys added or removed) during the iteration.
  //

  typedef unsigned char Iterator;

  /**
   * Get a iterator that represents the first key in the node, and which can be used to iterate through the node.
   *
   * @return an iterator pointing to the first key in the node.
   */
  inline const Iterator *first() const { return _wildcards._data; }

  /**
   * Advance the iterator to the next key in the node..
   *
   * @param it The current position in the iteration.
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND at the end of the iteration, another error code otherwise.
   */
  Error next(const Iterator **it) const;

  /**
   * Determine whether the iteration has ended.
   *
   * @param it The current position in the iteration.
   * @return true if a key exists for the position, false if the position points to the end of the node.
   */
  inline bool last(const Iterator *it) const { return it ? !*it : true; }

  /**
   * Get the key associated with an iterator.
   *
   * @param it The current it in the iteration.
   * @param key Will point to the key which will not be NULL terminated
   * @param keySize The size of the key
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND at the end of the iteration, another error code otherwise.
   */
  inline Error key(const Iterator *it, const char **key, UInt32 *keySize) const {
    if (!key || !keySize || !it) {
      return ESB_NULL_POINTER;
    }

    const unsigned char *p = it;

    if (!*p) {
      return ESB_CANNOT_FIND;
    }

    UInt8 size = (UInt8)*p++;
    *keySize = size;
    *key = (const char *)p;
    return ESB_SUCCESS;
  }

  /**
   * Get the value associated with an iterator.
   *
   * @param it The current it in the iteration.
   * @param value The value associated with the iterator
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND at the end of the iteration, another error code otherwise.
   */
  Error value(const Iterator *it, SmartPointer &value) const {
    const unsigned char *p = it;

    if (!*p) {
      return ESB_CANNOT_FIND;
    }

    UInt8 size = *p;
    value = (ReferenceCount *)ReadPointer(p + size + 1);
    return ESB_SUCCESS;
  }

  inline bool empty() const { return 0 == _wildcards._data[0] && (!_extra._data || 0 == _extra._data[0]); }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

  /** Placement new
   *
   * @param size The size of the memory block
   * @param block A valid memory block with which the object can be constructed.
   * @return The memory block
   */
  inline void *operator new(size_t size, unsigned char *block) noexcept { return block; }

 private:
  unsigned char *find(const SizedBuffer &buffer, const char *key, UInt32 keySize, bool *exists) const;
  void clear(const SizedBuffer &buffer);
  static WildcardIndexNode *Create(const char *key, int keyLength, unsigned char *block);

  char *_key;              // NULL terminated, fixed.
  SizedBuffer _wildcards;  // fixed storage for wildcards
  SizedBuffer _extra;      // growable storage for additional wildcards

  // must use static Create().
  WildcardIndexNode();

  ESB_DISABLE_AUTO_COPY(WildcardIndexNode);
};

/**
 * An index suitable for performing wildcard matches as described in RFC 2818 and other RFCs.  Wildcard patterns can
 * have a single '*' which can occur anywhere in the leftmost component of the fqdn (e.g,. "f*.bar.com", "*o.bar.com",
 * "f*o.bar.com" and "foo.bar.com" are all valid for this index, but patterns like "foo.*.com" or "*o*.bar.com" are not
 * supported).
 *
 * The index also does most-specific matching.  Here 'most-specific' means the match which matches the least number of
 * characters against the wildcard.  In cases where there are multiple best matches, one will be picked
 * non-deterministically.
 *
 * Wildcard patterns like "*.bar.com" do not match "bar.com".
 *
 * RFC 2818 3.1:
 *
 * If more than one identity of a given type is present in
 * the certificate (e.g., more than one dNSName name, a match in any one
 * of the set is considered acceptable.) Names may contain the wildcard
 * character * which is considered to match any single domain name
 * component or component fragment. E.g., *.a.com matches foo.a.com but
 * not bar.foo.a.com. f*.com matches foo.com but not bar.com.
 */
class WildcardIndex : public EmbeddedMapBase {
 public:
  WildcardIndex(UInt32 numBuckets, UInt32 numLocks, Allocator &allocator);
  virtual ~WildcardIndex();

  /**
   * Add a new wildcard or exact match pattern to the index
   *
   * @param domain The "bar.com" in "f*o.bar.com"
   * @param wildcard The "f*o" in "f*o.bar.com"
   * @param value The smart pointer value.  The reference count will be increased by one while it resides in the index.
   * @param updateIfExists if the domain+wildcard already exists, update the smart pointer to point to the new value
   * @return ESB_SUCCESS if successful, ESB_UNIQUENESS_VIOLATION if the domain+wildcard already exists and
   * updateIfExists is false (the default), another error code otherwise.
   */
  Error insert(const char *domain, const char *wildcard, SmartPointer &value, bool updateIfExists = false);

  /**
   * Remove a wildcard or exact match pattern from the index.  If a wildcard is removed, the reference count of the
   * associated smart pointer will be decremented.
   *
   * @param domain The "bar.com" in "f*o.bar.com"
   * @param wildcard The "f*o" in "f*o.bar.com"
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the domain+wildcard were not in the index, another error code
   * otherwise.
   */
  Error remove(const char *domain, const char *wildcard);

  /**
   * Update the smart pointer value associated with a wildcard or exact match pattern.
   *
   * @param domain The "bar.com" in "f*o.bar.com"
   * @param wildcard The "f*o" in "f*o.bar.com"
   * @param value The smart pointer value.  The reference count will be increased by one while it resides in the index.
   * @param old If non-NULL and the wildcard was found in the index, will be set to the previously associated value.
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the domain+wildcard were not in the index, another error code
   * otherwise.
   */
  Error update(const char *domain, const char *wildcard, SmartPointer &value, SmartPointer *old = NULL);

  /**
   * Find the smart pointer value associated with a wildcard or exact match pattern - THIS DOES NOT EVALUATE WILDCARD
   * PATTERNS.
   *
   * @param domain The "bar.com" in "f*o.bar.com"
   * @param wildcard The "f*o" in "f*o.bar.com"
   * @param value The smart pointer value will be stored here on success.
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the domain+wildcard were not in the index, another error code
   * otherwise.
   */
  Error find(const char *domain, const char *wildcard, SmartPointer &value);

  /**
   * Evaluate a hostname against all wildcard patterns for the domain and, if any match, return the most specific match.
   * In the case of a tie (multiple patterns match with the same degree of specificity), the match will be
   * non-deterministic.
   *
   * @param domain The "bar.com" in "f*o.bar.com"
   * @param hostname The "foo" to match against "f*o.bar.com"
   * @param value The smart pointer value will be stored here on success.
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if no wildcards in the index matched the hostname, another error
   * code otherwise.
   */
  Error match(const char *domain, const char *hostname, SmartPointer &value);

  /**
   * Remove all wildcards from the index.
   */
  inline void clear() {
    EmbeddedMapBase::clear();  // moves all nodes in _map to the _deadNodes list.
  }

 private:
  class WildcardIndexCallbacks : public EmbeddedMapCallbacks {
   public:
    WildcardIndexCallbacks(Mutex &lock, EmbeddedList &deadNodes) : _lock(lock), _deadNodes(deadNodes) {}
    virtual ~WildcardIndexCallbacks(){};

    virtual int compare(const void *f, const void *s) const;
    virtual UInt32 hash(const void *key) const;
    virtual void cleanup(EmbeddedMapElement *element);

   private:
    Mutex &_lock;
    EmbeddedList &_deadNodes;
  };

  inline Lockable &bucketLock(UInt32 bucket) const {
    return 0 == _numBucketLocks ? (Lockable &)NullLock::Instance() : _bucketLocks[bucket % _numBucketLocks];
  }

  Mutex _deadNodesLock;
  EmbeddedList _deadNodes;
  WildcardIndexCallbacks _callbacks;
  UInt32 _numBucketLocks;
  ReadWriteLock *_bucketLocks;

  ESB_DISABLE_AUTO_COPY(WildcardIndex);
};

}  // namespace ESB

#endif
