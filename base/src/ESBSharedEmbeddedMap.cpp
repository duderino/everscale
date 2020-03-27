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

HashComparator::HashComparator() {}

HashComparator::~HashComparator() {}

SharedEmbeddedMap::SharedEmbeddedMap(HashComparator &comparator,
                                     UInt32 numBuckets, UInt32 numLocks,
                                     Allocator &allocator)
    : _numElements(),
      _numBuckets(numBuckets),
      _numLocks(ESB_MIN(numBuckets, numLocks)),
      _comparator(comparator),
      _buckets(NULL),
      _locks(NULL),
      _allocator(allocator) {
  _buckets =
      (EmbeddedList *)_allocator.allocate(numBuckets * sizeof(EmbeddedList));
  if (!_buckets) {
    return;
  }

  _locks = (Mutex *)_allocator.allocate(numLocks * sizeof(Mutex));
  if (!_locks) {
    allocator.deallocate(_buckets);
    _buckets = NULL;
    return;
  }

  // call ctors directly on the memory

  for (UInt32 i = 0; i < numBuckets; ++i) {
    new (&_buckets[i]) EmbeddedList();
  }

  for (UInt32 i = 0; i < numLocks; ++i) {
    new (&_locks[i]) Mutex();
  }
}

SharedEmbeddedMap::~SharedEmbeddedMap() {
  if (_buckets) {
    for (UInt32 i = 0; i < _numBuckets; ++i) {
      while (true) {
        EmbeddedListElement *elem = _buckets[i].removeFirst();
        if (!elem) {
          break;
        }

        CleanupHandler *handler = elem->cleanupHandler();
        if (handler) {
          handler->destroy(elem);
        }
      }

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

  UInt32 bucket = _comparator.hash(value->key()) % _numBuckets;
  WriteScopeLock lock(_locks[bucket % _numLocks]);
  _buckets[bucket].addFirst(value);
  _numElements.inc();

  return ESB_SUCCESS;
}

EmbeddedMapElement *SharedEmbeddedMap::remove(const void *key) {
  if (!_buckets || !key) {
    return NULL;
  }

  UInt32 bucket = _comparator.hash(key) % _numBuckets;
  WriteScopeLock lock(_locks[bucket % _numLocks]);

  EmbeddedMapElement *elem = (EmbeddedMapElement *)_buckets[bucket].first();
  for (; elem; elem = (EmbeddedMapElement *)elem->next()) {
    if (0 == _comparator.compare(key, elem->key())) {
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

  UInt32 bucket = _comparator.hash(key) % _numBuckets;
  ReadScopeLock lock(_locks[bucket % _numLocks]);

  EmbeddedMapElement *elem = (EmbeddedMapElement *)_buckets[bucket].first();
  for (; elem; elem = (EmbeddedMapElement *)elem->next()) {
    if (0 == _comparator.compare(key, elem->key())) {
      return elem;
    }
  }

  return NULL;
}

bool SharedEmbeddedMap::validate(double *chiSquared) const {
  double expectedLength = ((double)_numElements.get()) / _numBuckets;
  double sum = 0.0;

  for (UInt32 i = 0; i < _numBuckets; ++i) {
    ReadScopeLock lock(_locks[i % _numLocks]);
    if (!_buckets[i].validate()) {
      return false;
    }
    if (chiSquared) {
      double actualLength = _buckets[i].size();
      sum += (actualLength - expectedLength) * (actualLength - expectedLength) /
             expectedLength;
    }
  }

  if (chiSquared) {
    *chiSquared = sum;
  }
  return true;
}

}  // namespace ESB
