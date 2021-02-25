#ifndef ESB_COMPACT_STRING_MAP_H
#include <ESBCompactStringMap.h>
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

#ifdef ESB_64BIT
static inline void *ReadPointer(const unsigned char *buffer) {
  UInt64 address = ((UInt64)*buffer++) << 56;
  address |= ((UInt64)*buffer++) << 48;
  address |= ((UInt64)*buffer++) << 40;
  address |= ((UInt64)*buffer++) << 32;
  address |= ((UInt64)*buffer++) << 24;
  address |= ((UInt64)*buffer++) << 16;
  address |= ((UInt64)*buffer++) << 8;
  address |= ((UInt64)*buffer);
  return (void *)address;
}
#else
static inline void *ReadPointer(const unsigned char *buffer) {
  UInt64 address = ((UInt32)*buffer++) << 24;
  address |= ((UInt64)*buffer++) << 16;
  address |= ((UInt64)*buffer++) << 8;
  address |= ((UInt64)*buffer);
  return (void *)address;
}
#endif

#ifdef ESB_64BIT
static inline void WritePointer(unsigned char *buffer, void *pointer) {
  UInt64 address = (UInt64)pointer;
  *buffer++ = address >> 56;
  *buffer++ = address >> 48;
  *buffer++ = address >> 40;
  *buffer++ = address >> 32;
  *buffer++ = address >> 24;
  *buffer++ = address >> 16;
  *buffer++ = address >> 8;
  *buffer++ = address;
}
#else
static inline void WritePointer(unsigned char *buffer, void *pointer) {
  UInt32 address = (UInt32)pointer;
  *buffer++ = address >> 24;
  *buffer++ = address >> 16;
  *buffer++ = address >> 8;
  *buffer++ = address;
}
#endif

CompactStringMap::CompactStringMap(ESB::UInt32 initialCapacity) : _buffer(NULL), _capacity(initialCapacity) {}

CompactStringMap::~CompactStringMap() {
  if (_buffer) {
    free(_buffer);
  }
}

Error CompactStringMap::insert(const char *key, UInt32 keySize, void *value, bool updateIfExists) {
  if (!key) {
    return ESB_NULL_POINTER;
  }

  if (keySize < 1) {
    return ESB_UNDERFLOW;
  }

  if (keySize > ESB_UINT8_MAX) {
    return ESB_OVERFLOW;
  }

  if (!_buffer) {
    _buffer = (unsigned char *)malloc(_capacity);
    if (!_buffer) {
      return ESB_OUT_OF_MEMORY;
    }
    memset(_buffer, 'a', _capacity);
    _buffer[0] = 0;
  }

  // Find the insertion point
  unsigned char *p = _buffer;
  while (true) {
    if (!*p) {
      break;
    }

    // Get length
    ESB::UInt8 size = (ESB::UInt8)*p;
    ++p;

    // if key exists, update or fail
    if (size == keySize && 0 == memcmp(p, key, size)) {
      if (!updateIfExists) {
        return ESB_UNIQUENESS_VIOLATION;
      }
      WritePointer(p + size, value);
      return ESB_SUCCESS;
    }

    // skip to next key
    p += size + sizeof(void *);
    assert(_capacity >= p - _buffer);
  }

  // Key doesn't exist
  assert(!*p);

  // Ensure space for new key
  UInt32 freeSpace = _capacity - (p - _buffer);
  if (freeSpace < keySize) {
    UInt32 requestSize = MAX(_capacity * 2, _capacity + keySize + sizeof(void *) + 1 - freeSpace);
    unsigned char *buffer = (unsigned char *)realloc(_buffer, requestSize);
    if (!buffer) {
      return ESB_OUT_OF_MEMORY;
    }
    p = buffer + (p - _buffer);
    assert(!*p);
    _buffer = (unsigned char *)buffer;
    _capacity = requestSize;
  }

  // Add key to the end
  *p++ = keySize;
  memcpy(p, key, keySize);
  p += keySize;
  WritePointer(p, value);
  p += sizeof(value);
  *p = 0;

  return ESB_SUCCESS;
}

Error CompactStringMap::remove(const char *key, UInt32 keySize) {
  if (!key) {
    return ESB_NULL_POINTER;
  }

  if (!_buffer) {
    return ESB_CANNOT_FIND;
  }

  unsigned char *p = _buffer;
  while (true) {
    if (!*p) {
      return ESB_CANNOT_FIND;
    }

    // Get length
    ESB::UInt8 size = (ESB::UInt8)*p;
    if (size == keySize && 0 == memcmp(p + 1, key, size)) {
      break;
    }

    // skip to next key
    p += size + sizeof(void *) + 1;
    assert(_capacity >= p - _buffer);
  }

  unsigned char *nextKey = p + keySize + sizeof(void *) + 1;
  UInt32 promoteSize = _capacity - (nextKey - _buffer);
  memmove(p, nextKey, promoteSize);
  return ESB_SUCCESS;
}

void *CompactStringMap::find(const char *key, UInt32 keySize) const {
  if (!key || !_buffer) {
    return NULL;
  }

  const unsigned char *p = _buffer;
  while (true) {
    if (!*p) {
      return NULL;
    }

    // Get length
    ESB::UInt8 size = (ESB::UInt8)*p;
    ++p;

    if (size == keySize && 0 == memcmp(p, key, size)) {
      return ReadPointer(p + size);
    }

    // skip to next key
    p += size + sizeof(void *);
    assert(_capacity >= p - _buffer);
  }
}

Error CompactStringMap::update(const char *key, UInt32 keySize, void *value, void **old) {
  if (!key) {
    return ESB_NULL_POINTER;
  }

  if (keySize < 1) {
    return ESB_UNDERFLOW;
  }

  if (keySize > ESB_UINT8_MAX) {
    return ESB_OVERFLOW;
  }

  if (!_buffer) {
    return ESB_CANNOT_FIND;
  }

  unsigned char *p = _buffer;
  while (true) {
    if (!*p) {
      return ESB_CANNOT_FIND;
    }

    // Get length
    ESB::UInt8 size = (ESB::UInt8)*p;
    ++p;

    if (size == keySize && 0 == memcmp(p, key, size)) {
      if (old) {
        *old = ReadPointer(p + size);
      }
      WritePointer(p + size, value);
      return ESB_SUCCESS;
    }

    // skip to next key
    p += size + sizeof(void *);
    assert(_capacity >= p - _buffer);
  }
}

Error CompactStringMap::clear() {
  if (_buffer) {
    _buffer[0] = 0;
  }
  return ESB_SUCCESS;
}

Error CompactStringMap::next(const char **key, UInt32 *keySize, void **value, UInt32 *marker) const {
  if (!key || !keySize || !value || !marker) {
    return ESB_NULL_POINTER;
  }

  if (!_buffer) {
    return ESB_CANNOT_FIND;
  }

  const unsigned char *p = _buffer + *marker;
  if (!*p) {
    return ESB_CANNOT_FIND;
  }

  // Get length
  ESB::UInt8 size = (ESB::UInt8)*p;
  *keySize = size;

  ++p;
  *key = (const char *)p;
  p += size;
  *value = ReadPointer(p);
  *marker = p + sizeof(void *) - _buffer;

  return ESB_SUCCESS;
}

}  // namespace ESB
