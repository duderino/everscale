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

#ifndef ESB_NULL_LOCK_H
#include <ESBNullLock.h>
#endif

#ifndef ESB_EMBEDDED_MAP_BASE_H
#include <ESBEmbeddedMapBase.h>
#endif

namespace ESB {

/** A hash table supporting concurrent access.  Uniqueness not enforced.
 *
 *  @ingroup util
 */
class SharedEmbeddedMap : public EmbeddedMapBase {
 public:
  /** Constructor.
   *
   * @param callbacks element comparison and cleanup functions.
   * @param numBuckets more buckets, fewer collisions, more memory.
   * @param numLocks more locks, less contention, more memory.  if 0, no
   * internal locking will be performed
   */
  SharedEmbeddedMap(EmbeddedMapCallbacks &callbacks, UInt32 numBuckets, UInt32 numLocks,
                    Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.  No cleanup handlers are called
   */
  virtual ~SharedEmbeddedMap();

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
  const EmbeddedMapElement *find(const void *key);

  inline int size() const { return _numElements.get(); }

  inline void clear() { EmbeddedMapBase::clear(); }

  /** Validate the map - for tests only.
   *
   * @param chiSquared If not NULL, the chiSquared statistic will be written
   * here if the function returns true.
   * @return true if the map is valid, false otherwise
   */
  inline bool validate(double *chiSquared) const { return EmbeddedMapBase::validate(chiSquared); }

 private:
  inline ESB::Lockable &bucketLock(ESB::UInt32 bucket) const {
    return 0 == _numBucketLocks ? (ESB::Lockable &)ESB::NullLock::Instance() : _bucketLocks[bucket % _numBucketLocks];
  }

  ESB::UInt32 _numBucketLocks;
  ESB::Mutex *_bucketLocks;

  ESB_DEFAULT_FUNCS(SharedEmbeddedMap);
};

}  // namespace ESB

#endif
