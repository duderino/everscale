#ifndef ESB_WILDCARD_INDEX_H
#define ESB_WILDCARD_INDEX_H

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_SMART_POINTER_H
#include <ESBSmartPointer.h>
#endif

#ifndef ESB_STRING_H
#include <ESBString.h>
#endif

#ifndef ESB_EMBEDDED_MAP_BASE_H
#include <ESBEmbeddedMapBase.h>
#endif

#ifndef ESB_SHARED_EMBEDDED_LIST_H
#include <ESBSharedEmbeddedList.h>
#endif

#ifndef ESB_READ_WRITE_LOCK_H
#include <ESBReadWriteLock.h>
#endif

#ifndef ESB_NULL_LOCK_H
#include <ESBNullLock.h>
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
   * @param key The string key to remove
   * @param keySize The size (length) of the string key
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key cannot be found, another error code otherwise.
   */
  Error remove(const char *key, UInt32 keySize);

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

 private:
  unsigned char *find(const SizedBuffer &buffer, const char *key, UInt32 keySize, bool *exists) const;
  void clear(const SizedBuffer &buffer);
  static WildcardIndexNode *Create(const char *key, int keyLength, unsigned char *block);

  char *_key;              // NULL terminated, fixed.
  SizedBuffer _wildcards;  // fixed storage for wildcards
  SizedBuffer _extra;      // growable storage for additional wildcards

  // must use static Create().
  WildcardIndexNode();

  ESB_DEFAULT_FUNCS(WildcardIndexNode);
};

/**
 * An index of smart pointers which implements requirements 1-3 of RFC 6125 wildcard certificate matching.
 *
 * From https://tools.ietf.org/html/rfc6125#section-6.4.3:
 *
 * 6.4.3.  Checking of Wildcard Certificates
 *
 *   A client employing this specification's rules MAY match the reference
 *   identifier against a presented identifier whose DNS domain name
 *   portion contains the wildcard character '*' as part or all of a label
 *   (following the description of labels and domain names in
 *   [DNS-CONCEPTS]).
 *
 *   For information regarding the security characteristics of wildcard
 *   certificates, see Section 7.2.
 *
 *   If a client matches the reference identifier against a presented
 *   identifier whose DNS domain name portion contains the wildcard
 *   character '*', the following rules apply:
 *
 *   1.  The client SHOULD NOT attempt to match a presented identifier in
 *       which the wildcard character comprises a label other than the
 *       left-most label (e.g., do not match bar.*.example.net).
 *
 *   2.  If the wildcard character is the only character of the left-most
 *       label in the presented identifier, the client SHOULD NOT compare
 *       against anything but the left-most label of the reference
 *       identifier (e.g., *.example.com would match foo.example.com but
 *       not bar.foo.example.com or example.com).
 *
 *   3.  The client MAY match a presented identifier in which the wildcard
 *       character is not the only character of the label (e.g.,
 *       baz*.example.net and *baz.example.net and b*z.example.net would
 *       be taken to match baz1.example.net and foobaz.example.net and
 *       buzz.example.net, respectively).  However, the client SHOULD NOT
 *       attempt to match a presented identifier where the wildcard
 *       character is embedded within an A-label or U-label [IDNA-DEFS] of
 *       an internationalized domain name [IDNA-PROTO].
 */
class WildcardIndex : public EmbeddedMapBase {
 public:
  /**
   * Construct a new index.
   *
   * @param numBuckets Number of buckets for internal hash table.  More buckets -> more memory, fewer collisions.
   * @param numLocks Number of locks for internal hash table.  If 0, no locking.  Else, more locks -> more memory, less
   * contention.
   * @param allocator The allocator to use for allocating internal buckets and nodes.
   */
  WildcardIndex(UInt32 numBuckets, UInt32 numLocks, Allocator &allocator);

  virtual ~WildcardIndex();

  /**
   * Add a new wildcard or exact match pattern to the index
   *
   * @param domain The "bar.com" in "f*o.bar.com".  Must be NULL-terminated
   * @param wildcard The "f*o" in "f*o.bar.com"
   * @param wildcardSize The size of the wildcard not including terminating NULL
   * @param value The smart pointer value.  The reference count will be increased by one while it resides in the index.
   * @param updateIfExists if the domain+wildcard already exists, update the smart pointer to point to the new value
   * @return ESB_SUCCESS if successful, ESB_UNIQUENESS_VIOLATION if the domain+wildcard already exists and
   * updateIfExists is false (the default), another error code otherwise.
   */
  Error insert(const char *domain, const char *wildcard, UInt32 wildcardSize, SmartPointer &value,
               bool updateIfExists = false);

  /**
   * Remove a wildcard or exact match pattern from the index.  If a wildcard is removed, the reference count of the
   * associated smart pointer will be decremented.
   *
   * @param domain The "bar.com" in "f*o.bar.com"
   * @param wildcard The "f*o" in "f*o.bar.com"
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the domain+wildcard were not in the index, another error code
   * otherwise.
   */
  Error remove(const char *domain, const char *wildcard, UInt32 wildcardSize);

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
  Error update(const char *domain, const char *wildcard, UInt32 wildcardSize, SmartPointer &value,
               SmartPointer *old = NULL);

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
  Error find(const char *domain, const char *wildcard, UInt32 wildcardSize, SmartPointer &value);

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
  Error match(const char *domain, const char *hostname, UInt32 hostnameSize, SmartPointer &value);

  /**
   * Remove all wildcards from the index.
   */
  inline void clear() {
    EmbeddedMapBase::clear();  // moves all nodes in _map to the _deadNodes list.
  }

  Allocator &allocator() { return _allocator; }

 private:
  class WildcardIndexCallbacks : public EmbeddedMapCallbacks {
   public:
    WildcardIndexCallbacks(SharedEmbeddedList &deadNodes) : _deadNodes(deadNodes) {}
    virtual ~WildcardIndexCallbacks(){};

    virtual int compare(const void *f, const void *s) const;
    virtual UInt64 hash(const void *key) const;
    virtual void cleanup(EmbeddedMapElement *element);

   private:
    SharedEmbeddedList &_deadNodes;

    ESB_DISABLE_AUTO_COPY(WildcardIndexCallbacks);
  };

  inline Lockable &bucketLock(UInt32 bucket) const {
    return 0 == _numBucketLocks ? (Lockable &)NullLock::Instance() : _bucketLocks[bucket % _numBucketLocks];
  }

  SharedEmbeddedList _deadNodes;
  WildcardIndexCallbacks _callbacks;
  UInt32 _numBucketLocks;
  ReadWriteLock *_bucketLocks;

  ESB_DEFAULT_FUNCS(WildcardIndex);
};

}  // namespace ESB

#endif
