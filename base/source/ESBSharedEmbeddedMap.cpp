#ifndef ESB_SHARED_EMBEDDED_MAP_H
#include <ESBSharedEmbeddedMap.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

#ifndef ESB_READ_SCOPE_LOCK_H
#include <ESBReadScopeLock.h>
#endif

#ifndef ESB_NULL_LOCK_H
#include <ESBNullLock.h>
#endif

namespace ESB {

SharedEmbeddedMap::Callbacks::Callbacks() {}
SharedEmbeddedMap::Callbacks::~Callbacks() {}

SharedEmbeddedMap::SharedEmbeddedMap(Callbacks &callbacks, UInt32 numBuckets, UInt32 numLocks, Allocator &allocator)
    : _numElements(),
      _numBuckets(numBuckets),
      _numLocks(MIN(numBuckets, numLocks)),
      _callbacks(callbacks),
      _buckets(NULL),
      _locks(NULL),
      _allocator(allocator) {
  _buckets = (EmbeddedList *)_allocator.allocate(numBuckets * sizeof(EmbeddedList));
  if (!_buckets) {
    return;
  }

  for (UInt32 i = 0; i < numBuckets; ++i) {
    new (&_buckets[i]) EmbeddedList();
  }

  if (0 < _numLocks) {
    _locks = (Mutex *)_allocator.allocate(numLocks * sizeof(Mutex));
    if (!_locks) {
      allocator.deallocate(_buckets);
      _buckets = NULL;
      return;
    }

    for (UInt32 i = 0; i < numLocks; ++i) {
      new (&_locks[i]) Mutex();
    }
  }
}

SharedEmbeddedMap::~SharedEmbeddedMap() {
  clear();

  if (_buckets) {
    for (UInt32 i = 0; i < _numBuckets; ++i) {
      _buckets[i].~EmbeddedList();
    }
    _allocator.deallocate(_buckets);
  }

  if (_locks) {
    for (UInt32 i = 0; i < _numLocks; ++i) {
      _locks[i].~Mutex();
    }
    _allocator.deallocate(_locks);
  }
}

void SharedEmbeddedMap::clear() {
  for (UInt32 i = 0; i < _numBuckets; ++i) {
    WriteScopeLock lock(bucketLock(i));
    while (true) {
      EmbeddedMapElement *element = (EmbeddedMapElement *)_buckets[i].removeFirst();
      if (!element) {
        break;
      }
      _callbacks.cleanup(element);
    }
  }
}

Lockable &SharedEmbeddedMap::bucketLock(ESB::UInt32 bucket) const {
  if (0 >= _numLocks) {
    return ESB::NullLock::Instance();
  }

  return _locks[bucket % _numLocks];
}

Error SharedEmbeddedMap::insert(EmbeddedMapElement *value) {
  if (!_buckets) {
    return ESB_OUT_OF_MEMORY;
  }

  if (!value) {
    return ESB_NULL_POINTER;
  }

  if (!value->key()) {
    return ESB_INVALID_ARGUMENT;
  }

  UInt32 bucket = _callbacks.hash(value->key()) % _numBuckets;
  WriteScopeLock lock(bucketLock(bucket));
  _buckets[bucket].addFirst(value);
  _numElements.inc();

  return ESB_SUCCESS;
}

EmbeddedMapElement *SharedEmbeddedMap::remove(const void *key) {
  if (!_buckets || !key) {
    return NULL;
  }

  UInt32 bucket = _callbacks.hash(key) % _numBuckets;
  WriteScopeLock lock(bucketLock(bucket));

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

const EmbeddedMapElement *SharedEmbeddedMap::find(const void *key) const {
  if (!_buckets || !key) {
    return NULL;
  }

  UInt32 bucket = _callbacks.hash(key) % _numBuckets;
  WriteScopeLock lock(bucketLock(bucket));

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

bool SharedEmbeddedMap::validate(double *chiSquared) const {
  double expectedLength = ((double)_numElements.get()) / _numBuckets;
  double sum = 0.0;

  for (UInt32 i = 0; i < _numBuckets; ++i) {
    ReadScopeLock lock(bucketLock(i));
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

}  // namespace ESB
