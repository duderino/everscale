#ifndef ESB_SHARED_EMBEDDED_MAP_H
#define ESB_SHARED_EMBEDDED_MAP_H

#ifndef ESB_EMBEDDED_MAP_ELEMENT_H
#include <ESBEmbeddedMapElement.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_COMPARATOR_H
#include <ESBComparator.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

namespace ESB {

/** A hash table supporting concurrent access
 *
 *  @ingroup util
 */
class SharedEmbeddedMap {
 public:
  /**
   * All the callbacks needed to customize a SharedEmbeddedMap
   */
  class Callbacks : public HashComparator {
   public:
    /**
     * Default constructor
     */
    Callbacks();

    virtual ~Callbacks();

    virtual void cleanup(EmbeddedMapElement *element) = 0;
  };

  /** Constructor.
   *
   * @param callbacks element comparison and cleanup functions.
   * @param numBuckets more buckets, fewer collisions, more memory.
   * @param numLocks more locks, less contention, more memory.  if 0, no
   * internal locking will be performed
   */
  SharedEmbeddedMap(Callbacks &callbacks, UInt32 numBuckets, UInt32 numLocks,
                    Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.  No cleanup handlers are called
   */
  virtual ~SharedEmbeddedMap();

  /** Remove all elements from the map.
   */
  void clear();

  /** Insert a key/value pair into the map.  O(1).
   *
   *  @param value The value to insert.  The value's getKey() function must
   * return non-NULL.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error insert(EmbeddedMapElement *value);

  /** Remove a key/value pair from the map given its key.  O(1).
   *
   *  @param key The key of to remove.
   *  @return The value associated with the first occurrence of the key, or NULL
   * if the key couldn't be found.
   */
  EmbeddedMapElement *remove(const void *key);

  /** Find a value in the map given its key.  O(1).
   *
   *  @param key The key of the key/value pair to find.
   *  @return The value associated with the first occurrence of the key, or NULL
   * if the key couldn't be found.
   */
  const EmbeddedMapElement *find(const void *key) const;

  inline int size() const { return _numElements.get(); }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

  /** Validate the map - for tests only.
   *
   * @param chiSquared If not NULL, the chiSquared statistic will be written
   * here if the function returns true.
   * @return true if the map is valid, false otherwise
   */
  bool validate(double *chiSquared) const;

 private:
  // Disabled
  SharedEmbeddedMap(const SharedEmbeddedMap &);
  SharedEmbeddedMap &operator=(const SharedEmbeddedMap &);

  Lockable &bucketLock(ESB::UInt32 bucket) const;

  SharedInt _numElements;
  UInt32 _numBuckets;
  UInt32 _numLocks;
  Callbacks &_callbacks;
  EmbeddedList *_buckets;
  mutable Mutex *_locks;
  Allocator &_allocator;
};

}  // namespace ESB

#endif
