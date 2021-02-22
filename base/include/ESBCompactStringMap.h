#ifndef ESB_COMPACT_STRING_MAP_H
#define ESB_COMPACT_STRING_MAP_H

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ESB {

/** A cache-line friendly uniquely associative array with string keys and void * values.
 *
 *  @ingroup util
 */
class CompactStringMap {
 public:
  CompactStringMap(UInt32 initialCapacity);

  virtual ~CompactStringMap();

  /** Insert a key/value pair into the map.  O(n).
   *
   *  @param key The key to insert.
   *  @param value The value to insert.  The value may be NULL.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   *      ESB_UNIQUENESS_VIOLATION will be returned if the key already
   *      exists and the map enforces uniqueness.
   */

  /**
   *
   * @param key
   * @param value
   * @param position
   * @return
   */
  Error insert(const char *key, void *value, int *position = NULL);

  /**
   *
   * @param key
   * @param keySize
   * @param value
   * @param position
   * @return
   */
  Error insert(const char *key, int keySize, void *value, int *position = NULL);

  /** Erase a key/value pair from the map given its key.  O(n).
   *  <p>
   *  Note, this will only deallocate memory used by the map's internal nodes.
   *  </p>
   *
   *  @param key The key of the key/value pair to erase.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   *      ESB_CANNOT_FIND will be returned if the key cannot be found.
   */

  /**
   *
   * @param key
   * @return
   */
  Error remove(const char *key);

  /**
   *
   * @param key
   * @param keySize
   * @return
   */
  Error remove(const char *key, int keySize);

  /**
   *
   * @param position
   * @return
   */
  Error remove(int position);

  /** Find a value in the map given its key.  O(n).
   *
   *  @param key The key of the key/value pair to find.
   *  @return The value or NULL if the value cannot be found.
   */

  /**
   *
   * @param key
   * @return
   */
  void *find(const char *key) const;

  /**
   *
   * @param key
   * @param keySize
   * @return
   */
  void *find(const char *key, int keySize) const;

  /**
   *
   * @param position
   * @return
   */
  void *find(int position) const;

  /** Update key/value pair in the map given its key.  O(n).
   *
   *  @param key The key of the key/value pair.
   *  @param value A new value to assign to the key/value pair.  Any resources
   *      used by the old value will not be released by this method.  The
   *      value may be NULL.
   *  @param old If non-NULL, the old value will be assigned to this.
   *  @return ESB_SUCCESS if successful, ESB_CANNOT_FIND if the key cannot
   *      be found, another error code otherwise.
   */

  /**
   *
   * @param key
   * @param value
   * @param old
   * @return
   */
  Error update(const char *key, void *value, void **old);

  /**
   *
   * @param key
   * @param keySize
   * @param value
   * @param old
   * @return
   */
  Error update(const char *key, int keySize, void *value, void **old);

  /**
   *
   * @param position
   * @param value
   * @param old
   * @return
   */
  Error update(int position, void *value, void **old);

  /** Remove all key/value pairs from the map.  O(1).
   *  <p>
   *  This will only deallocate memory used by the map's internal nodes.
   *  </p>
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   *      Clearing an empty map will return ESB_SUCCESS.
   */
  Error clear();

  inline UInt32 firstMarker() const { return 0; }

  inline bool hasNext(UInt32 marker) const { return _buffer[marker] != 0; }

  /**
   *
   * @param key
   * @param keySize
   * @param value
   * @param position
   * @return
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
  UInt32 _totalCapacity;
  UInt32 _usedCapacity;
  char *_buffer;

  ESB_DISABLE_AUTO_COPY(CompactStringMap);
};

}  // namespace ESB

#endif
