#ifndef ESB_SHARED_EMBEDDED_MAP_H
#include <ESBSharedEmbeddedMap.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

#ifndef ESB_READ_SCOPE_LOCK_H
#include <ESBReadScopeLock.h>
#endif

namespace ESB {

EmbeddedMapCallbacks::EmbeddedMapCallbacks() {}
EmbeddedMapCallbacks::~EmbeddedMapCallbacks() {}

EmbeddedMapBase::EmbeddedMapBase(EmbeddedMapCallbacks &callbacks, UInt32 numBuckets, Allocator &allocator)
    : _numElements(), _numBuckets(numBuckets), _callbacks(callbacks), _buckets(NULL), _allocator(allocator) {
  _buckets = (EmbeddedList *)_allocator.allocate(numBuckets * sizeof(EmbeddedList));
  if (!_buckets) {
    return;
  }

  for (UInt32 i = 0; i < numBuckets; ++i) {
    new (&_buckets[i]) EmbeddedList();
  }
}

EmbeddedMapBase::~EmbeddedMapBase() {
  clear();

  if (_buckets) {
    for (UInt32 i = 0; i < _numBuckets; ++i) {
      _buckets[i].~EmbeddedList();
    }
    _allocator.deallocate(_buckets);
  }
}

void EmbeddedMapBase::clear() {
  for (UInt32 i = 0; i < _numBuckets; ++i) {
    while (true) {
      EmbeddedMapElement *element = (EmbeddedMapElement *)_buckets[i].removeFirst();
      if (!element) {
        break;
      }
      _callbacks.cleanup(element);
    }
  }
}

EmbeddedMapElement *EmbeddedMapBase::find(ESB::UInt32 bucket, const void *key) {
  if (!_buckets) {
    return NULL;
  }

  EmbeddedMapElement *elem = (EmbeddedMapElement *)_buckets[bucket].first();
  for (; elem; elem = (EmbeddedMapElement *)elem->next()) {
    if (0 == _callbacks.compare(key, elem->key())) {
      break;
    }
  }

  if (elem && _buckets[bucket].first() != elem) {
    _buckets[bucket].remove(elem);
    _buckets[bucket].addFirst(elem);
  }

  return elem;
}

Error EmbeddedMapBase::insert(ESB::UInt32 bucket, EmbeddedMapElement *value) {
  if (!_buckets) {
    return ESB_OUT_OF_MEMORY;
  }

  _buckets[bucket].addFirst(value);
  _numElements.inc();
  return ESB_SUCCESS;
}

EmbeddedMapElement *EmbeddedMapBase::remove(ESB::UInt32 bucket, const void *key) {
  if (!_buckets) {
    return NULL;
  }

  EmbeddedMapElement *elem = (EmbeddedMapElement *)_buckets[bucket].first();
  for (; elem; elem = (EmbeddedMapElement *)elem->next()) {
    if (0 == _callbacks.compare(key, elem->key())) {
      _buckets[bucket].remove(elem);
      _numElements.dec();
      return elem;
    }
  }

  return NULL;
}

bool EmbeddedMapBase::validate(double *chiSquared) const {
  double expectedLength = ((double)_numElements.get()) / _numBuckets;
  double sum = 0.0;

  for (UInt32 i = 0; i < _numBuckets; ++i) {
    if (!_buckets[i].validate()) {
      return false;
    }
    if (chiSquared) {
      double actualLength = _buckets[i].size();
      sum += (actualLength - expectedLength) * (actualLength - expectedLength) / expectedLength;
    }
  }

  if (chiSquared) {
    *chiSquared = sum;
  }

  return true;
}

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
