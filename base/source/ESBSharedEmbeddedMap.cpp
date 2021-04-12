#ifndef ESB_SHARED_EMBEDDED_MAP_H
#include <ESBSharedEmbeddedMap.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

namespace ESB {

SharedEmbeddedMap::SharedEmbeddedMap(EmbeddedMapCallbacks &callbacks, UInt32 numBuckets, UInt32 numLocks,
                                     Allocator &allocator)
    : EmbeddedMapBase(callbacks, numBuckets, allocator),
      _numBucketLocks(MIN(numBuckets, numLocks)),
      _bucketLocks(NULL) {
  if (0 < _numBucketLocks) {
    _bucketLocks = (Mutex *)_allocator.allocate(numLocks * sizeof(Mutex));
    if (!_bucketLocks) {
      return;  // subsequent interactions with this object will return ESB_OUT_OF_MEMORY
    }

    for (UInt32 i = 0; i < numLocks; ++i) {
      new (&_bucketLocks[i]) Mutex();
    }
  }
}

SharedEmbeddedMap::~SharedEmbeddedMap() {
  if (_bucketLocks) {
    for (ESB::UInt32 i = 0; i < _numBucketLocks; ++i) {
      _bucketLocks[i].~Mutex();
    }
    _allocator.deallocate(_bucketLocks);
  }
}

Error SharedEmbeddedMap::insert(EmbeddedMapElement *value) {
  if (!value) {
    return ESB_NULL_POINTER;
  }

  if (!value->key()) {
    return ESB_INVALID_ARGUMENT;
  }

  if (0 < _numBucketLocks && !_bucketLocks) {
    return ESB_OUT_OF_MEMORY;
  }

  UInt32 bucket = EmbeddedMapBase::bucket(value->key());
  WriteScopeLock lock(bucketLock(bucket));
  return EmbeddedMapBase::insert(bucket, value);
}

EmbeddedMapElement *SharedEmbeddedMap::remove(const void *key) {
  if (!key) {
    return NULL;
  }

  if (0 < _numBucketLocks && !_bucketLocks) {
    return NULL;
  }

  UInt32 bucket = EmbeddedMapBase::bucket(key);
  WriteScopeLock lock(bucketLock(bucket));
  return EmbeddedMapBase::remove(bucket, key);
}

const EmbeddedMapElement *SharedEmbeddedMap::find(const void *key) {
  if (!key) {
    return NULL;
  }

  if (0 < _numBucketLocks && !_bucketLocks) {
    return NULL;
  }

  UInt32 bucket = EmbeddedMapBase::bucket(key);
  WriteScopeLock lock(bucketLock(bucket));
  return EmbeddedMapBase::find(bucket, key);
}

}  // namespace ESB
