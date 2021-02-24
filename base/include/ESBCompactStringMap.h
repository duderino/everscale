#ifndef ESB_COMPACT_STRING_MAP_H
#define ESB_COMPACT_STRING_MAP_H

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ESB {

/** A cache-line friendly uniquely associative array with string keys and void * values.  Note that this collection uses
 * malloc/free/realloc internally and does not use the allocator framework.  Note also that keys must be at least 1 byte
 * and at most 255 bytes.
 *
 *  @ingroup util
 */
class CompactStringMap {
 public:
  CompactStringMap(ESB::UInt32 initialCapacity = 1024);

  virtual ~CompactStringMap();

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
  inline Error insert(const char *key, void *value, bool updateIfExists = false) {
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
  Error insert(const char *key, int keySize, void *value, bool updateIfExists = false);

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
  Error remove(const char *key, int keySize);

  /**
   * Return the value associated with a string key, if it can be found.  O(n)
   *
   * @param key The NULL-terminated string key to look for
   * @return The value associated with the key if found, NULL otherwise.
   */
  inline void *find(const char *key) const { return key ? find(key, strlen(key)) : NULL; }

  /**
   * Return the value associated with a string key, if it can be found.  O(n)
   *
   * @param key The string key to look for
   * @param keySize The size (length) of the string key
   * @return The value associated with the key if found, NULL otherwise.
   */
  void *find(const char *key, int keySize) const;

  /**
   * Update a string key with a new pointer.  O(n)
   *
   * @param key The NULL-terminated string key to update
   * @param keySize The size (length) of the string key
   * @param value The pointer to associate with the string key
   * @param old If not NULL, set to the key's associated value before the update.
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key doesn't exist, another error code otherwise.
   */
  inline Error update(const char *key, void *value, void **old) {
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
  Error update(const char *key, int keySize, void *value, void **old);

  /** Remove all key/value pairs from the map.  O(1).
   *  <p>
   *  This will only deallocate memory used by the map's internal nodes.
   *  </p>
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error clear();

  inline UInt32 firstMarker() const { return 0; }

  inline bool hasNext(UInt32 marker) const { return _buffer ? _buffer[marker] != 0 : false; }

  /**
   * Iterate through all string key + value pairs in the map.  O(n).
   *
   * @param key Will point to the key which will not be NULL terminated
   * @param keySize The size of the key
   * @param value The value associated with the key
   * @param marker The marker set by the prior call or firstMarker().
   * @return ESB_SUCCESS if successful, ESB_CANNOT_FIND at the end of the iteration, another error code otherwise.
   */
  Error next(const char **key, int *keySize, void **value, UInt32 *marker) const;

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
  unsigned char *_buffer;
  UInt32 _capacity;

  ESB_DISABLE_AUTO_COPY(CompactStringMap);
};

}  // namespace ESB

#endif
