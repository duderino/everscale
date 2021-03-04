#ifndef ES_WILDCARD_INDEX_H
#define ES_WILDCARD_INDEX_H

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

namespace ES {

/** A cache-line friendly uniquely associative array with string keys and void * values.  Note that this collection uses
 * malloc/free/realloc internally and does not use the allocator framework.  Note also that keys must be at least 1 byte
 * and at most 255 bytes.
 *
 *  @ingroup util
 */
class WildcardIndexNode : public ESB::EmbeddedMapElement {
 public:
  static WildcardIndexNode *Create(const char *key, ESB::Allocator &allocator = ESB::SystemAllocator::Instance());

  virtual ~WildcardIndexNode();

  virtual ESB::CleanupHandler *cleanupHandler();

  virtual const void *key() const;

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
  inline ESB::Error insert(const char *key, ESB::SmartPointer &value, bool updateIfExists = false) {
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
  ESB::Error insert(const char *key, ESB::UInt32 keySize, ESB::SmartPointer &value, bool updateIfExists = false);

  /**
   * Remove a string key from the map.  O(n)
   *
   * @param key The NULL-terminated string key to remove
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key cannot be found, another error code otherwise.
   */
  inline ESB::Error remove(const char *key) { return key ? remove(key, strlen(key)) : ESB_NULL_POINTER; }

  /**
   * Remove a string key from the map.  O(n)
   *
   * @param key The string key to remove
   * @param keySize The size (length) of the string key
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key cannot be found, another error code otherwise.
   */
  ESB::Error remove(const char *key, ESB::UInt32 keySize);

  /**
   * Return the value associated with a string key, if it can be found.  O(n)
   *
   * @param key The NULL-terminated string key to look for
   * @param value Will point to the value if found
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key cannot be found, another error code otherwise.
   */
  inline ESB::Error find(const char *key, ESB::SmartPointer &value) const {
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
  ESB::Error find(const char *key, ESB::UInt32 keySize, ESB::SmartPointer &value) const;

  /**
   * Update a string key with a new pointer.  O(n)
   *
   * @param key The NULL-terminated string key to update
   * @param keySize The size (length) of the string key
   * @param value The pointer to associate with the string key
   * @param old If not NULL, set to the key's associated value before the update.
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key doesn't exist, another error code otherwise.
   */
  inline ESB::Error update(const char *key, ESB::SmartPointer &value, ESB::SmartPointer *old) {
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
  ESB::Error update(const char *key, ESB::UInt32 keySize, ESB::SmartPointer &value, ESB::SmartPointer *old);

  /** Remove all key/value pairs from the map.  O(1).
   */
  void clear();

  typedef unsigned char Marker;

  /**
   * Get a 'marker' that represents the first key in the map, and which can be used to iterate through the collection
   * using hasNext() and next().
   *
   * @return the marker for the first key in the map.
   */
  inline const Marker *firstMarker() const { return _wildcards._data; }

  /**
   * Determine whether a key exists for the current marker.
   *
   * @param marker The marker / current position in the iteration.
   * @return true if a key exists for the marker, false if the marker points to the end of the map.
   */
  inline bool hasNext(const Marker *marker) const { return marker ? *marker : false; }

  /**
   * Iterate through all string key + value pairs in the map.  O(n).  The map must not be modified during the iteration.
   *
   * @param key Will point to the key which will not be NULL terminated
   * @param keySize The size of the key
   * @param value The value associated with the key
   * @param marker The marker set by the prior call or firstMarker().
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND at the end of the iteration, another error code otherwise.
   */
  ESB::Error next(const char **key, ESB::UInt32 *keySize, ESB::SmartPointer &value, const Marker **marker) const;

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept { return allocator.allocate(size); }

  /** Placement new
   *
   * @param size The size of the memory block
   * @param block A valid memory block with which the object can be constructed.
   * @return The memory block
   */
  inline void *operator new(size_t size, unsigned char *block) noexcept { return block; }

 private:
  unsigned char *find(const ESB::SizedBuffer &buffer, const char *key, ESB::UInt32 keySize, bool *exists) const;
  void clear(const ESB::SizedBuffer &buffer);

  char *_key;                   // NULL terminated, fixed.
  ESB::SizedBuffer _wildcards;  // fixed storage for wildcards
  ESB::SizedBuffer _extra;      // growable storage for additional wildcards

  // must use static Create().
  WildcardIndexNode();

  ESB_DISABLE_AUTO_COPY(WildcardIndexNode);
};

}  // namespace ES

#endif
