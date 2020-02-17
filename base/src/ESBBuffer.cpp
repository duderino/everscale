#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#else
#error "Need stdio.h or equivalent"
#endif

#if defined HAVE_STDARG_H
#include <stdarg.h>
#else
#error "Need stdarg.h or equivalent"
#endif

namespace ESB {

Buffer::Buffer(unsigned char *buffer, unsigned int capacity)
    : _readMark(0),
      _writeMark(0),
      _readPosition(0),
      _writePosition(0),
      _capacity(capacity),
      _buffer(buffer) {}

Buffer::~Buffer() {}

CleanupHandler *Buffer::getCleanupHandler() { return 0; }

Buffer *Buffer::Create(Allocator *allocator, unsigned int capacity) {
  if (0 == allocator || 1 > capacity) {
    return 0;
  }

  unsigned char *block = (unsigned char *)allocator->allocate(
      capacity * sizeof(unsigned char) + ESB_WORD_ALIGN(sizeof(Buffer)));

  if (!block) {
    return 0;
  }

  return new (block) Buffer(block + ESB_WORD_ALIGN(sizeof(Buffer)),
                            capacity * sizeof(unsigned char));
}

unsigned char *Buffer::duplicate(Allocator *allocator, bool trim) const {
  unsigned char *dup = 0;
  int length = getWritePosition();

  if (trim) {
    for (int i = length - 1; i >= 0; --i) {
      if (' ' == _buffer[i] || '\t' == _buffer[i] || '\n' == _buffer[i] ||
          '\r' == _buffer[i]) {
        --length;
        continue;
      }

      break;
    }
  }

  dup =
      (unsigned char *)allocator->allocate(length * sizeof(unsigned char) + 1);

  if (0 == dup) {
    return 0;
  }

  if (0 < length) {
    memcpy(dup, _buffer, length);
  }

  dup[length] = 0;

  return dup;
}

bool Buffer::match(const unsigned char *str) const {
  return 0 == strncmp((const char *)_buffer + _readPosition, (const char *)str,
                      _writePosition - _readPosition);
}

bool Buffer::compact() {
  if (0 == _readPosition) {
    return false;
  }

  if (_writePosition != _readPosition) {
#ifdef HAVE_MEMMOVE
    memmove(_buffer, _buffer + _readPosition, _writePosition - _readPosition);
#else
#error "memmove or equivalent is required"
#endif
  }

  _writePosition -= _readPosition;
  _readPosition = 0;
  _readMark = 0;
  _writeMark = 0;

  return true;
}

}  // namespace ESB
