/** @file ESFBuffer.cpp
 *  @brief A general purpose buffer with parsing and formatting utilities.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_BUFFER_H
#include <ESFBuffer.h>
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

ESFBuffer::ESFBuffer(unsigned char *buffer, unsigned int capacity)
    : _readMark(0),
      _writeMark(0),
      _readPosition(0),
      _writePosition(0),
      _capacity(capacity),
      _buffer(buffer) {}

ESFBuffer::~ESFBuffer() {}

ESFCleanupHandler *ESFBuffer::getCleanupHandler() { return 0; }

ESFBuffer *ESFBuffer::Create(ESFAllocator *allocator, unsigned int capacity) {
  if (0 == allocator || 1 > capacity) {
    return 0;
  }

  unsigned char *block = (unsigned char *)allocator->allocate(
      capacity * sizeof(unsigned char) + ESF_WORD_ALIGN(sizeof(ESFBuffer)));

  if (!block) {
    return 0;
  }

  return new (block) ESFBuffer(block + ESF_WORD_ALIGN(sizeof(ESFBuffer)),
                               capacity * sizeof(unsigned char));
}

unsigned char *ESFBuffer::duplicate(ESFAllocator *allocator, bool trim) const {
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

bool ESFBuffer::match(const unsigned char *str) const {
  return 0 == strncmp((const char *)_buffer + _readPosition, (const char *)str,
                      _writePosition - _readPosition);
}

bool ESFBuffer::compact() {
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
