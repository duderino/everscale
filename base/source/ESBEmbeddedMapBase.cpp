#ifndef ESB_EMBEDDED_MAP_BASE_H
#include <ESBEmbeddedMapBase.h>
#endif

namespace ESB {

EmbeddedMapCallbacks::EmbeddedMapCallbacks() {}
EmbeddedMapCallbacks::~EmbeddedMapCallbacks() {}

EmbeddedMapBase::EmbeddedMapBase(EmbeddedMapCallbacks &callbacks, UInt32 numBuckets, Allocator &allocator)
    : _numElements(), _numBuckets(numBuckets), _callbacks(callbacks), _buckets(NULL), _allocator(allocator) {
  if (0 == numBuckets) {
    return;
  }

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

}  // namespace ESB
