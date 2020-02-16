/** @file ESFBuffer.h
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
#define ESF_BUFFER_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_EMBEDDED_LIST_ELEMENT_H
#include <ESFEmbeddedListElement.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#error "Need string.h or equivalent"
#endif

/** A general purpose buffer with parsing and formatting utilities.
 *
 *  @ingroup util
 */
class ESFBuffer : public ESFEmbeddedListElement {
 public:
  /** Constructor. Create a ESFBuffer wrapper for a C buffer.
   *  Note that the C buffer will not be freed in the ESFBuffer's
   *  destructor.
   *
   *  The buffer will start in fill mode.
   *
   * @param buffer The buffer to wrap
   * @param capacity The size in bytes of the buffer to wrap.
   */
  ESFBuffer(unsigned char *buffer, unsigned int capacity);

  /** Destructor.
   */
  virtual ~ESFBuffer();

  /** Allocate a new buffer of a given size.  The actual amount of data
   *  allocated will be the size + the buffer object and a few extra bytes.
   *
   * @param allocator the Allocator to create the buffer with
   * @param capacity The byte size of the buffer's internal buffer
   * @return A newly created buffer object or NULL if sufficient memory could
   *  not be allocated.
   */
  static ESFBuffer *Create(ESFAllocator *allocator, unsigned int capacity);

  /** Return an optional handler that can destroy the element.
   *
   * @return A handler to destroy the element or NULL if the element should not
   * be destroyed.
   */
  virtual ESFCleanupHandler *getCleanupHandler();

  /** Get the total storage capacity of the buffer (used + unused space).
   *
   *  @return the total capacity of the buffer
   */
  inline unsigned int getCapacity() const { return _capacity; }

  /** Get the remaining number of characters that can be written (put).
   *
   *  @return the remaining number of characters
   */
  inline unsigned int getWritable() const { return _capacity - _writePosition; }

  /** Get the remaining number of characters that can be read (get).
   *
   *  @return the remaining number of characters
   */
  inline unsigned int getReadable() const {
    return _writePosition - _readPosition;
  }

  /** Determine whether there are remaining characters to be read.
   *
   * @return whether there are remaining characters/space
   */
  inline bool isReadable() const { return 0 < getReadable(); }

  /** Determine whether there is space to write additional characters.
   *
   *
   * @return whether there are remaining characters/space
   */
  inline bool isWritable() const { return 0 < getWritable(); }

  /** Write a character to the buffer and advance the write position.  If the
   * buffer is full this is a no-op.
   *
   * @param character The character to be added to the buffer.
   */
  inline void putNext(unsigned char character) {
    if (false == isWritable()) {
      return;
    }

    _buffer[_writePosition++] = character;
  }

  /** Read a character from the buffer and advance the read position.  If the
   * buffer is empty this is a no-op.
   *
   * @return The next character or 0 if there are no more characters
   */
  inline unsigned char getNext() {
    return isReadable() ? _buffer[_readPosition++] : 0;
  }

  /** Read the next character in the buffer but do not advance the read
   * position.  If the buffer is empty this is a no-op.
   *
   * @return The next character or 0 if there are no more characters.
   */
  inline unsigned char peekNext() const {
    return isReadable() ? _buffer[_readPosition] : 0;
  }

  /** Skip the next character in the buffer.  If
   *  the buffer is empty this is a no-op.  Buffer should be in drain mode.
   */
  inline void skipNext() {
    _readPosition = false == isReadable() ? _readPosition : _readPosition + 1;
  }

  /** Save the current read position
   */
  inline void readMark() { _readMark = _readPosition; }

  /** Revert to a saved read position
   */
  inline void readReset() { _readPosition = _readMark; }

  /** Save the current write position
   */
  inline void writeMark() { _writeMark = _writePosition; }

  /** Revert to a saved write position
   */
  inline void writeReset() { _writePosition = _writeMark; }

  /** Get the current write position.
   *
   * @return The current write position
   */
  inline unsigned int getWritePosition() const { return _writePosition; }

  /** Get the current read position.
   *
   * @return The current read position
   */
  inline unsigned int getReadPosition() const { return _readPosition; }

  /** Set the current write position
   *
   * @param position The new value for the write position
   */
  inline void setWritePosition(unsigned int position) {
    _writePosition = position > _capacity ? _capacity : position;
  }

  /** Set the current read position
   *
   * @param position The new value for the read position
   */
  inline void setReadPosition(unsigned int position) {
    _readPosition = position > _capacity ? _capacity : position;
  }

  /** Get the underlying buffer.
   *
   * @return The underlying buffer
   */
  inline unsigned char *getBuffer() { return _buffer; }

  /** Get the underlying buffer.
   *
   * @return The underlying buffer
   */
  inline const unsigned char *getBuffer() const { return _buffer; }

  /** Advance the read position n steps up to the write position, but no more.
   *
   * @param n The number of steps to advance the read position
   */
  inline void skip(unsigned int n) {
    _readPosition =
        _readPosition + n > _writePosition ? _writePosition : _readPosition + n;
  }

  /** Clear the buffer.  Both the read and write positions will be set to zero.
   * Also clears any marks.
   */
  inline void clear() {
    _readPosition = 0;
    _writePosition = 0;
    _readMark = 0;
    _writeMark = 0;
  }

  /** Discard all read data so more space can be used.
   *
   * @return true if additional space was made available, false otherwise
   */
  bool compact();

  /** Duplicate the data in the buffer.  This will duplicate everything from the
   * 0th position to the write position.  The duplicate will be null terminated.
   *
   * @param allocator The memory for the duplicate will be taken from this
   * allocator
   * @param trim If true, any trailing whitespace (' ', '\t', '\r', '\n') will
   * be trimmed
   * @return The duplicate or NULL if memory could not be allocated.
   */
  unsigned char *duplicate(ESFAllocator *allocator, bool trim) const;

  /** Duplicate the data in the buffer.  This will duplicate everything from the
   * 0th position to the write position.  The duplicate will be null terminated
   * and any trailing whitespace (' ', '\t', '\r', '\n') will be trimmed.
   *
   * @param allocator The memory for the duplicate will be taken from this
   * allocator
   * @return The duplicate or NULL if memory could not be allocated.
   */
  inline unsigned char *duplicate(ESFAllocator *allocator) const {
    return duplicate(allocator, true);
  }

  /** Compare a string to data in the buffer.  If in drain mode, this will
   * compare to unread data.  If in fill mode, this will compare to the data
   * that has already been written to the buffer.
   *
   * @param str The string to compare
   * @return true if equal, false otherwise
   */
  bool match(const unsigned char *str) const;

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

  /** Placement new
   *
   * @param size The size of the memory block
   * @param block A valid memory block with which the object can be constructed.
   * @return The memory block
   */
  inline void *operator new(size_t size, void *block) { return block; }

 private:
  // Disabled
  ESFBuffer(const ESFBuffer &);
  ESFBuffer &operator=(const ESFBuffer &);

  unsigned int _readMark;
  unsigned int _writeMark;
  unsigned int _readPosition;
  unsigned int _writePosition;
  unsigned int _capacity;
  unsigned char *_buffer;
};

#endif /* ! ESF_BUFFER_H */
