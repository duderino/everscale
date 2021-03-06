#ifndef ES_WILDCARD_INDEX_H
#include <ESWildcardIndex.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#error "Need string.h or equivalent"
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

namespace ES {

static const ESB::UInt32 DefaultAlloc = ESB_MAX_HOSTNAME + 3 * sizeof(ESB::SmartPointer);

WildcardIndexNode *WildcardIndexNode::Create(const char *key, ESB::Allocator &allocator) {
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

  ESB::UInt32 size = sizeof(WildcardIndexNode) + DefaultAlloc;
  unsigned char *block = (unsigned char *)allocator.allocate(size);

  WildcardIndexNode *node = new (block) WildcardIndexNode();
  node->_key = (char *)block + sizeof(WildcardIndexNode);
  memcpy(node->_key, key, length);
  node->_key[length] = 0;
  node->_wildcards._data = (unsigned char *)node->_key + length + 1;
  node->_wildcards._capacity = size - sizeof(WildcardIndexNode) - length - 1;
  node->_extra._data = NULL;
  node->_extra._capacity = 0;

  return node;
}

WildcardIndexNode::WildcardIndexNode() {
  // See Create() for most of the initialization
}

WildcardIndexNode::~WildcardIndexNode() { clear(); }

unsigned char *WildcardIndexNode::find(const ESB::SizedBuffer &buffer, const char *key, ESB::UInt32 keySize,
                                       bool *exists) const {
  // Find the insertion point
  unsigned char *p = buffer._data;
  while (true) {
    if (!*p) {
      break;
    }

    // Get length
    ESB::UInt8 size = (ESB::UInt8)*p;

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

ESB::Error WildcardIndexNode::insert(const char *key, ESB::UInt32 keySize, ESB::SmartPointer &value,
                                     bool updateIfExists) {
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
    ESB::SmartPointer ptr = (ESB::ReferenceCount *)ESB::ReadPointer(p + keySize + 1);
    ptr->dec();
    value->inc();
    ESB::WritePointer(p + keySize + 1, value.raw());
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
      ESB::SmartPointer ptr = (ESB::ReferenceCount *)ESB::ReadPointer(p + keySize + 1);
      ptr->dec();
      value->inc();
      ESB::WritePointer(q + keySize, value.raw());
      return ESB_SUCCESS;
    }
    assert(0 == *q);
  }

  // Key not found.  If _wildcards has enough free space, prefer it for key storage
  ESB::UInt32 freeSpace = _wildcards._capacity - (p - _wildcards._data);
  if (freeSpace > keySize) {
    *p++ = keySize;
    memcpy(p, key, keySize);
    p += keySize;
    value->inc();
    ESB::WritePointer(p, value.raw());
    p += sizeof(value);
    *p = 0;
    return ESB_SUCCESS;
  }

  // use _extra spillover area, (re)allocating memory if necessary

  if (!_extra._data) {
    _extra._data = (unsigned char *)malloc(MAX(DefaultAlloc, keySize + sizeof(ESB::SmartPointer) + 1));
    if (!_extra._data) {
      return ESB_OUT_OF_MEMORY;
    }
    _extra._data[0] = 0;
    _extra._capacity = DefaultAlloc;
    q = _extra._data;
  }

  // Ensure space for new key
  freeSpace = _extra._capacity - (q - _extra._data);
  if (freeSpace <= keySize) {
    ESB::UInt32 requestSize =
        MAX(_extra._capacity * 2, _extra._capacity + keySize + sizeof(ESB::SmartPointer) + 1 - freeSpace);
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
  ESB::WritePointer(q, value.raw());
  q += sizeof(value);
  *q = 0;

  return ESB_SUCCESS;
}

ESB::Error WildcardIndexNode::remove(const char *key, ESB::UInt32 keySize) {
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
    ESB::SmartPointer ptr = (ESB::ReferenceCount *)ESB::ReadPointer(p + keySize + 1);
    ptr->dec();
    unsigned char *nextKey = p + keySize + sizeof(void *) + 1;
    ESB::UInt32 promoteSize = _wildcards._capacity - (nextKey - _wildcards._data);
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

  ESB::SmartPointer ptr = (ESB::ReferenceCount *)ESB::ReadPointer(p + keySize + 1);
  ptr->dec();
  unsigned char *nextKey = p + keySize + sizeof(void *) + 1;
  ESB::UInt32 promoteSize = _extra._capacity - (nextKey - _extra._data);
  memmove(p, nextKey, promoteSize);
  return ESB_SUCCESS;
}

ESB::Error WildcardIndexNode::find(const char *key, ESB::UInt32 keySize, ESB::SmartPointer &value) const {
  if (!key || 0 == keySize || ESB_UINT8_MAX < keySize) {
    return ESB_CANNOT_FIND;
  }

  // Find the key
  bool keyExists = false;
  const unsigned char *p = find(_wildcards, key, keySize, &keyExists);

  if (keyExists) {
    assert(keySize == *p);
    value = (ESB::ReferenceCount *)ESB::ReadPointer(p + keySize + 1);
    return ESB_SUCCESS;
  }
  assert(0 == *p);

  if (!_extra._data) {
    return ESB_CANNOT_FIND;
  }

  p = find(_extra, key, keySize, &keyExists);

  if (keyExists) {
    assert(keySize == *p);
    value = (ESB::ReferenceCount *)ESB::ReadPointer(p + keySize + 1);
    return ESB_SUCCESS;
  }

  assert(0 == *p);
  return ESB_CANNOT_FIND;
}

ESB::Error WildcardIndexNode::update(const char *key, ESB::UInt32 keySize, ESB::SmartPointer &value,
                                     ESB::SmartPointer *old) {
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
    ESB::SmartPointer ptr = (ESB::ReferenceCount *)ESB::ReadPointer(p + keySize + 1);
    if (old) {
      *old = ptr;
    }
    ptr->dec();
    value->inc();
    ESB::WritePointer(p + keySize + 1, value.raw());
    return ESB_SUCCESS;
  }
  assert(0 == *p);

  if (!_extra._data) {
    return ESB_CANNOT_FIND;
  }

  p = find(_extra, key, keySize, &keyExists);

  if (keyExists) {
    assert(keySize == *p);
    ESB::SmartPointer ptr = (ESB::ReferenceCount *)ESB::ReadPointer(p + keySize + 1);
    if (old) {
      *old = ptr;
    }
    ptr->dec();
    value->inc();
    ESB::WritePointer(p + keySize + 1, value.raw());
    return ESB_SUCCESS;
  }

  assert(0 == *p);
  return ESB_CANNOT_FIND;
}

void WildcardIndexNode::clear(const ESB::SizedBuffer &buffer) {
  if (!buffer._data) {
    return;
  }

  unsigned char *p = buffer._data;
  while (true) {
    if (!*p) {
      break;
    }

    ESB::UInt8 size = (ESB::UInt8)*p;

    // Decrement the stored pointer.  May free the referenced object once the ptr goes out of scope.
    ESB::SmartPointer ptr = (ESB::ReferenceCount *)ESB::ReadPointer(p + size + 1);
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

ESB::Error WildcardIndexNode::next(const Iterator **it) const {
  if (!it) {
    return ESB_NULL_POINTER;
  }

  const unsigned char *p = *it;

  if (!*p) {
    return ESB_CANNOT_FIND;
  }

  if (_wildcards._data <= p && p < _wildcards._data + _wildcards._capacity) {
    ESB::UInt8 size = (ESB::UInt8)*p++;
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
    ESB::UInt8 size = (ESB::UInt8)*p++;
    *it = p + size + sizeof(void *);
    return ESB_SUCCESS;
  }

  assert(!"invalid iterator");
  return ESB_INVALID_ARGUMENT;
}

ESB::CleanupHandler *WildcardIndexNode::cleanupHandler() { return NULL; }

const void *WildcardIndexNode::key() const { return _key; }

}  // namespace ES
