#ifndef ESB_WILDCARD_INDEX_H
#include <ESBWildcardIndex.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

#ifndef ESB_READ_SCOPE_LOCK_H
#include <ESBReadScopeLock.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#error "Need string.h or equivalent"
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

namespace ESB {

static const UInt32 DefaultAlloc = sizeof(WildcardIndexNode) + ESB_MAX_HOSTNAME + 3 * sizeof(void *);

WildcardIndexNode *WildcardIndexNode::Create(const char *key, int keyLength, unsigned char *block) {
  WildcardIndexNode *node = new (block) WildcardIndexNode();
  node->_key = (char *)block + sizeof(WildcardIndexNode);
  memcpy(node->_key, key, keyLength);
  node->_key[keyLength] = 0;
  node->_wildcards._data = (unsigned char *)node->_key + keyLength + 1;
  node->_wildcards._capacity = DefaultAlloc - sizeof(WildcardIndexNode) - keyLength - 1;
  node->_extra._data = NULL;
  node->_extra._capacity = 0;

  return node;
}

WildcardIndexNode *WildcardIndexNode::Create(const char *key, Allocator &allocator) {
  // TODO move validation logic up to WildcardIndex?
  assert(key);
  if (!key) {
    return NULL;
  }

  int length = strlen(key);
  assert(0 < length);
  assert(ESB_MAX_HOSTNAME >= length);
  if (0 >= length || ESB_MAX_HOSTNAME < length) {
    return NULL;
  }

  // Use one block of memory for node, node's key, and node's wildcards.

  unsigned char *block = (unsigned char *)allocator.allocate(DefaultAlloc);

  if (!block) {
    return NULL;
  }

  return Create(key, length, block);
}

WildcardIndexNode *WildcardIndexNode::Recycle(const char *key, EmbeddedListElement *element) {
  // TODO move validation logic up to WildcardIndex?
  assert(key);
  if (!key) {
    return NULL;
  }

  int length = strlen(key);
  assert(0 < length);
  assert(ESB_MAX_HOSTNAME >= length);
  if (0 >= length || ESB_MAX_HOSTNAME < length) {
    return NULL;
  }

  return Create(key, length, (unsigned char *)element);
}

WildcardIndexNode::WildcardIndexNode() {
  // See Create() for most of the initialization
}

WildcardIndexNode::~WildcardIndexNode() { clear(); }

unsigned char *WildcardIndexNode::find(const SizedBuffer &buffer, const char *key, UInt32 keySize, bool *exists) const {
  // Find the insertion point
  unsigned char *p = buffer._data;
  while (true) {
    if (!*p) {
      break;
    }

    // Get length
    UInt8 size = (UInt8)*p;

    // if key exists, update or fail
    if (size == keySize && 0 == memcmp(p + 1, key, size)) {
      *exists = true;
      return p;
    }

    // skip to next key
    p += size + sizeof(void *) + 1;
    assert(buffer._capacity >= p - buffer._data);
  }

  // Key doesn't exist
  assert(!*p);
  *exists = false;
  return p;
}

Error WildcardIndexNode::insert(const char *key, UInt32 keySize, SmartPointer &value, bool updateIfExists) {
  if (!key) {
    return ESB_NULL_POINTER;
  }

  if (keySize < 1) {
    return ESB_UNDERFLOW;
  }

  if (keySize > ESB_UINT8_MAX) {
    return ESB_OVERFLOW;
  }

  if (value.isNull()) {
    return ESB_INVALID_ARGUMENT;
  }

  // Find the insertion point in _wildcards
  bool keyExists = false;
  unsigned char *p = find(_wildcards, key, keySize, &keyExists);

  if (keyExists) {
    assert(keySize == *p);
    if (!updateIfExists) {
      return ESB_UNIQUENESS_VIOLATION;
    }
    SmartPointer ptr = (ReferenceCount *)ReadPointer(p + keySize + 1);
    ptr->dec();
    value->inc();
    WritePointer(p + keySize + 1, value.raw());
    return ESB_SUCCESS;
  }
  assert(0 == *p);

  // Find the insertion point in the _extra spillover buffer
  unsigned char *q = NULL;

  if (_extra._data) {
    q = find(_extra, key, keySize, &keyExists);

    if (keyExists) {
      assert(keySize == *q);
      if (!updateIfExists) {
        return ESB_UNIQUENESS_VIOLATION;
      }
      SmartPointer ptr = (ReferenceCount *)ReadPointer(p + keySize + 1);
      ptr->dec();
      value->inc();
      WritePointer(q + keySize, value.raw());
      return ESB_SUCCESS;
    }
    assert(0 == *q);
  }

  // Key not found.  If _wildcards has enough free space, prefer it for key storage
  UInt32 freeSpace = _wildcards._capacity - (p - _wildcards._data);
  if (freeSpace > keySize + sizeof(void *) + 2) {
    *p++ = keySize;
    memcpy(p, key, keySize);
    p += keySize;
    value->inc();
    WritePointer(p, value.raw());
    p += sizeof(value);
    *p = 0;
    return ESB_SUCCESS;
  }

  // use _extra spillover area, (re)allocating memory if necessary

  if (!_extra._data) {
    _extra._data = (unsigned char *)malloc(MAX(DefaultAlloc, keySize + sizeof(SmartPointer) + 1));
    if (!_extra._data) {
      return ESB_OUT_OF_MEMORY;
    }
    _extra._data[0] = 0;
    _extra._capacity = DefaultAlloc;
    q = _extra._data;
  }

  // Ensure space for new key
  freeSpace = _extra._capacity - (q - _extra._data);
  if (freeSpace <= keySize + sizeof(void *) + 2) {
    UInt32 requestSize = MAX(_extra._capacity * 2, _extra._capacity + keySize + sizeof(SmartPointer) + 1 - freeSpace);
    unsigned char *buffer = (unsigned char *)realloc(_extra._data, requestSize);
    if (!buffer) {
      return ESB_OUT_OF_MEMORY;
    }
    q = buffer + (q - _extra._data);
    assert(!*q);
    _extra._data = (unsigned char *)buffer;
    _extra._capacity = requestSize;
  }

  // Insert/overwrite key
  *q++ = keySize;
  memcpy(q, key, keySize);
  q += keySize;
  value->inc();
  WritePointer(q, value.raw());
  q += sizeof(value);
  *q = 0;

  return ESB_SUCCESS;
}

Error WildcardIndexNode::remove(const char *key, UInt32 keySize) {
  if (!key) {
    return ESB_NULL_POINTER;
  }

  if (0 == keySize) {
    return ESB_INVALID_ARGUMENT;
  }

  if (ESB_UINT8_MAX < keySize) {
    return ESB_CANNOT_FIND;
  }

  // Find the key
  bool keyExists = false;
  unsigned char *p = find(_wildcards, key, keySize, &keyExists);

  if (keyExists) {
    assert(keySize == *p);
    SmartPointer ptr = (ReferenceCount *)ReadPointer(p + keySize + 1);
    ptr->dec();
    unsigned char *nextKey = p + keySize + sizeof(void *) + 1;
    UInt32 promoteSize = _wildcards._capacity - (nextKey - _wildcards._data);
    memmove(p, nextKey, promoteSize);
    return ESB_SUCCESS;
  }
  assert(0 == *p);

  if (!_extra._data) {
    return ESB_CANNOT_FIND;
  }

  // Find the key
  p = find(_extra, key, keySize, &keyExists);

  if (!keyExists) {
    assert(0 == *p);
    return ESB_CANNOT_FIND;
  }
  assert(keySize == *p);

  SmartPointer ptr = (ReferenceCount *)ReadPointer(p + keySize + 1);
  ptr->dec();
  unsigned char *nextKey = p + keySize + sizeof(void *) + 1;
  UInt32 promoteSize = _extra._capacity - (nextKey - _extra._data);
  memmove(p, nextKey, promoteSize);
  return ESB_SUCCESS;
}

Error WildcardIndexNode::find(const char *key, UInt32 keySize, SmartPointer &value) const {
  if (!key || 0 == keySize || ESB_UINT8_MAX < keySize) {
    return ESB_CANNOT_FIND;
  }

  // Find the key
  bool keyExists = false;
  const unsigned char *p = find(_wildcards, key, keySize, &keyExists);

  if (keyExists) {
    assert(keySize == *p);
    value = (ReferenceCount *)ReadPointer(p + keySize + 1);
    return ESB_SUCCESS;
  }
  assert(0 == *p);

  if (!_extra._data) {
    return ESB_CANNOT_FIND;
  }

  p = find(_extra, key, keySize, &keyExists);

  if (keyExists) {
    assert(keySize == *p);
    value = (ReferenceCount *)ReadPointer(p + keySize + 1);
    return ESB_SUCCESS;
  }

  assert(0 == *p);
  return ESB_CANNOT_FIND;
}

Error WildcardIndexNode::update(const char *key, UInt32 keySize, SmartPointer &value, SmartPointer *old) {
  if (!key) {
    return ESB_NULL_POINTER;
  }

  if (keySize < 1) {
    return ESB_UNDERFLOW;
  }

  if (keySize > ESB_UINT8_MAX) {
    return ESB_OVERFLOW;
  }

  if (value.isNull()) {
    return ESB_INVALID_ARGUMENT;
  }

  // Find the key
  bool keyExists = false;
  unsigned char *p = find(_wildcards, key, keySize, &keyExists);

  if (keyExists) {
    assert(keySize == *p);
    SmartPointer ptr = (ReferenceCount *)ReadPointer(p + keySize + 1);
    if (old) {
      *old = ptr;
    }
    ptr->dec();
    value->inc();
    WritePointer(p + keySize + 1, value.raw());
    return ESB_SUCCESS;
  }
  assert(0 == *p);

  if (!_extra._data) {
    return ESB_CANNOT_FIND;
  }

  p = find(_extra, key, keySize, &keyExists);

  if (keyExists) {
    assert(keySize == *p);
    SmartPointer ptr = (ReferenceCount *)ReadPointer(p + keySize + 1);
    if (old) {
      *old = ptr;
    }
    ptr->dec();
    value->inc();
    WritePointer(p + keySize + 1, value.raw());
    return ESB_SUCCESS;
  }

  assert(0 == *p);
  return ESB_CANNOT_FIND;
}

void WildcardIndexNode::clear(const SizedBuffer &buffer) {
  if (!buffer._data) {
    return;
  }

  unsigned char *p = buffer._data;
  while (true) {
    if (!*p) {
      break;
    }

    UInt8 size = (UInt8)*p;

    // Decrement the stored pointer.  May free the referenced object once the ptr goes out of scope.
    SmartPointer ptr = (ReferenceCount *)ReadPointer(p + size + 1);
    ptr->dec();

    // skip to next key
    p += size + sizeof(void *) + 1;
    assert(buffer._capacity >= p - buffer._data);
  }

  buffer._data[0] = 0;
}

void WildcardIndexNode::clear() {
  clear(_wildcards);
  if (_extra._data) {
    clear(_extra);
    free(_extra._data);
    _extra._data = NULL;
    _extra._capacity = 0U;
  }
}

Error WildcardIndexNode::next(const Iterator **it) const {
  if (!it) {
    return ESB_NULL_POINTER;
  }

  const unsigned char *p = *it;

  if (!*p) {
    return ESB_CANNOT_FIND;
  }

  if (_wildcards._data <= p && p < _wildcards._data + _wildcards._capacity) {
    UInt8 size = (UInt8)*p++;
    p += size + sizeof(void *);

    if (*p) {
      *it = p;
      return ESB_SUCCESS;
    }

    // No more keys in _wildcards, but set the iterator to the next key in _extra if it has been allocated.

    if (_extra._data) {
      *it = _extra._data;
      return ESB_SUCCESS;
    }

    *it = p;
    return ESB_SUCCESS;
  }

  assert(_extra._data);

  if (_extra._data <= p && p < _extra._data + _extra._capacity) {
    UInt8 size = (UInt8)*p++;
    *it = p + size + sizeof(void *);
    return ESB_SUCCESS;
  }

  assert(!"invalid iterator");
  return ESB_INVALID_ARGUMENT;
}

CleanupHandler *WildcardIndexNode::cleanupHandler() { return NULL; }

const void *WildcardIndexNode::key() const { return _key; }

int WildcardIndex::WildcardIndexCallbacks::compare(const void *f, const void *s) const {
  if (f == s) {
    return 0;
  }

  if (!f) {
    return -1;
  }

  if (!s) {
    return 1;
  }

  return strcmp((const char *)f, (const char *)s);
}

UInt32 WildcardIndex::WildcardIndexCallbacks::hash(const void *key) const { return StringHash((const char *)key); }

void WildcardIndex::WildcardIndexCallbacks::cleanup(EmbeddedMapElement *element) {
  element->~EmbeddedMapElement();
  _deadNodes.addFirst(element);
}

WildcardIndex::WildcardIndex(UInt32 numBuckets, UInt32 numLocks, Allocator &allocator)
    : EmbeddedMapBase(_callbacks, numBuckets, allocator),
      _deadNodes(),
      _callbacks(_deadNodes),
      _numBucketLocks(MIN(numBuckets, numLocks)),
      _bucketLocks(NULL) {
  if (0 < _numBucketLocks) {
    _bucketLocks = (ReadWriteLock *)_allocator.allocate(numLocks * sizeof(ReadWriteLock));
    if (!_bucketLocks) {
      return;  // subsequent interactions with this object will return ESB_OUT_OF_MEMORY
    }

    for (UInt32 i = 0; i < numLocks; ++i) {
      new (&_bucketLocks[i]) ReadWriteLock();
    }
  }
}

WildcardIndex::~WildcardIndex() {
  clear();
  for (EmbeddedListElement *e = _deadNodes.removeFirst(); e; e = _deadNodes.removeFirst()) {
    _allocator.deallocate(e);
  }

  if (_bucketLocks) {
    for (UInt32 i = 0; i < _numBucketLocks; ++i) {
      _bucketLocks[i].~ReadWriteLock();
    }
    _allocator.deallocate(_bucketLocks);
  }
}

Error WildcardIndex::insert(const char *domain, const char *wildcard, UInt32 wildcardSize, SmartPointer &value,
                            bool updateIfExists) {
  if (!domain || !wildcard) {
    return ESB_NULL_POINTER;
  }

  if (0 < _numBucketLocks && !_bucketLocks) {
    return ESB_OUT_OF_MEMORY;
  }

  UInt32 bucket = EmbeddedMapBase::bucket(domain);
  WriteScopeLock(bucketLock(bucket));
  WildcardIndexNode *node = (WildcardIndexNode *)EmbeddedMapBase::find(bucket, domain);

  if (node) {
    return node->insert(wildcard, wildcardSize, value, updateIfExists);
  }

  EmbeddedListElement *element = _deadNodes.removeFirst();

  node = element ? WildcardIndexNode::Recycle(domain, element) : WildcardIndexNode::Create(domain, _allocator);
  if (!node) {
    return ESB_OUT_OF_MEMORY;
  }

  Error error = node->insert(wildcard, wildcardSize, value, updateIfExists);
  if (ESB_SUCCESS != error) {
    _callbacks.cleanup(node);
    return error;
  }

  error = EmbeddedMapBase::insert(bucket, node);
  if (ESB_SUCCESS != error) {
    _callbacks.cleanup(node);
    return error;
  }

  return ESB_SUCCESS;
}

Error WildcardIndex::remove(const char *domain, const char *wildcard) {
  if (!domain || !wildcard) {
    return ESB_NULL_POINTER;
  }

  if (0 < _numBucketLocks && !_bucketLocks) {
    return ESB_OUT_OF_MEMORY;
  }

  UInt32 bucket = EmbeddedMapBase::bucket(domain);
  WriteScopeLock(bucketLock(bucket));
  WildcardIndexNode *node = (WildcardIndexNode *)EmbeddedMapBase::find(bucket, domain);

  if (!node) {
    return ESB_CANNOT_FIND;
  }

  Error error = node->remove(wildcard);
  if (ESB_SUCCESS != error) {
    return error;
  }

  if (node->empty()) {
    removeElement(bucket, node);
    _callbacks.cleanup(node);
  }

  return ESB_SUCCESS;
}

Error WildcardIndex::update(const char *domain, const char *wildcard, SmartPointer &value, SmartPointer *old) {
  if (!domain || !wildcard) {
    return ESB_NULL_POINTER;
  }

  if (0 < _numBucketLocks && !_bucketLocks) {
    return ESB_OUT_OF_MEMORY;
  }

  UInt32 bucket = EmbeddedMapBase::bucket(domain);
  WriteScopeLock(bucketLock(bucket));
  WildcardIndexNode *node = (WildcardIndexNode *)EmbeddedMapBase::find(bucket, domain);

  return node ? node->update(wildcard, value, old) : ESB_CANNOT_FIND;
}

Error WildcardIndex::find(const char *domain, const char *wildcard, SmartPointer &value) {
  if (!domain || !wildcard) {
    return ESB_NULL_POINTER;
  }

  if (0 < _numBucketLocks && !_bucketLocks) {
    return ESB_OUT_OF_MEMORY;
  }

  UInt32 bucket = EmbeddedMapBase::bucket(domain);
  ReadScopeLock(bucketLock(bucket));
  WildcardIndexNode *node = (WildcardIndexNode *)EmbeddedMapBase::find(bucket, domain);

  return node ? node->find(wildcard, value) : ESB_CANNOT_FIND;
}

Error WildcardIndex::match(const char *domain, const char *hostname, SmartPointer &value) {
  if (!domain || !hostname) {
    return ESB_NULL_POINTER;
  }

  if (0 < _numBucketLocks && !_bucketLocks) {
    return ESB_OUT_OF_MEMORY;
  }

  UInt32 bucket = EmbeddedMapBase::bucket(domain);
  ReadScopeLock(bucketLock(bucket));
  WildcardIndexNode *node = (WildcardIndexNode *)EmbeddedMapBase::find(bucket, domain);

  if (!node) {
    return ESB_CANNOT_FIND;
  }

  const WildcardIndexNode::Iterator *bestMatchIterator;
  int bestMatchValue = -1;
  const WildcardIndexNode::Iterator *it = node->first();
  UInt32 hostnameLength = strlen(hostname);

  while (true) {
    if (node->last(it)) {
      break;
    }

    const char *wildcard = NULL;
    UInt32 wildcardSize;

    node->key(it, &wildcard, &wildcardSize);
    int matchValue = StringWildcardMatch(wildcard, wildcardSize, hostname, hostnameLength);

    if (0 == matchValue) {
      // exact match
      node->value(it, value);
      return ESB_SUCCESS;
    }

    if (0 < matchValue && (0 > bestMatchValue || bestMatchValue > matchValue)) {
      bestMatchValue = matchValue;
      bestMatchIterator = it;
    }

    Error error = node->next(&it);
    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  if (0 > bestMatchValue) {
    return ESB_CANNOT_FIND;
  }

  node->value(bestMatchIterator, value);
  return ESB_SUCCESS;
}

}  // namespace ESB
