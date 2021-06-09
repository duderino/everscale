#ifndef ESB_EMBEDDED_MAP_BASE_H
#define ESB_EMBEDDED_MAP_BASE_H

#ifndef ESB_EMBEDDED_MAP_ELEMENT_H
#include <ESBEmbeddedMapElement.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
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

/**
 * All the callbacks needed to customize an embedded map
 */
class EmbeddedMapCallbacks : public HashComparator {
 public:
  /**
   * Default constructor
   */
  EmbeddedMapCallbacks();

  virtual ~EmbeddedMapCallbacks();

  virtual void cleanup(EmbeddedMapElement *element) = 0;

  ESB_DISABLE_AUTO_COPY(EmbeddedMapCallbacks);
};

/**
 * Not intended for direct use.
 */
class EmbeddedMapBase {
 public:
  virtual ~EmbeddedMapBase();

 protected:
  EmbeddedMapBase(EmbeddedMapCallbacks &callbacks, UInt32 numLocks, Allocator &allocator);

  inline UInt32 bucket(const void *key) const { return _callbacks.hash(key) % _numBuckets; }

  EmbeddedMapElement *find(ESB::UInt32 bucket, const void *key);

  Error insert(ESB::UInt32 bucket, EmbeddedMapElement *value);

  EmbeddedMapElement *remove(ESB::UInt32 bucket, const void *key);

  inline void removeElement(ESB::UInt32 bucket, EmbeddedMapElement *element) {
    if (_buckets) {
      _buckets[bucket].remove(element);
    }
  }

  void clear();

  bool validate(double *chiSquared) const;

  SharedInt _numElements;
  UInt32 _numBuckets;
  EmbeddedMapCallbacks &_callbacks;
  EmbeddedList *_buckets;
  Allocator &_allocator;

  ESB_DISABLE_AUTO_COPY(EmbeddedMapBase);
};

}  // namespace ESB

#endif
