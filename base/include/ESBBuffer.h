#ifndef ESB_BUFFER_H
#define ESB_BUFFER_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#error "Need string.h or equivalent"
#endif

#define ESB_BUFFER_OVERHEAD (ESB_WORD_ALIGN(sizeof(ESB::Buffer)))

namespace ESB {

/** A general purpose buffer with parsing and formatting utilities.
 *
 *  @ingroup util
 */
class Buffer : public EmbeddedListElement {
 public:
  /** Constructor. Create a Buffer wrapper for a C buffer.
   *  Note that the C buffer will not be freed in the Buffer's
   *  destructor.
   *
   *  The buffer will start in fill mode.
   *
   * @param buffer The buffer to wrap
   * @param capacity The size in bytes of the buffer to wrap.
   */
  Buffer(unsigned char *buffer, UInt32 capacity);

  /** Destructor.
   */
  virtual ~Buffer();

  /** Allocate a new buffer of a given size.  The actual amount of data
   *  allocated will be the size + the buffer object and alignment bytes
   *
   * @param allocator the Allocator to create the buffer with
   * @param capacitmy The byte size of the buffer's internal buffer
   * @return A newly created buffer object or NULL if sufficient memory could
   *  not be allocated.
   */
  static Buffer *Create(Allocator &allocator, UInt32 capacity);

  static void Destroy(Allocator &allocator, Buffer *buffer);

  /** Return an optional handler that can destroy the element.
   *
   * @return A handler to destroy the element or NULL if the element should not
   * be destroyed.
   */
  virtual CleanupHandler *cleanupHandler();

  /** Get the total storage capacity of the buffer (used + unused space).
   *
   *  @return the total capacity of the buffer
   */
  inline UInt32 capacity() const { return _capacity; }

  /** Get the remaining number of characters that can be written (put).
   *
   *  @return the remaining number of characters
   */
  inline UInt32 writable() const { return _capacity - _writePosition; }

  /** Get the remaining number of characters that can be read (get).
   *
   *  @return the remaining number of characters
   */
  inline UInt32 readable() const { return _writePosition - _readPosition; }

  /** Determine whether there are remaining characters to be read.
   *
   * @return whether there are remaining characters/space
   */
  inline bool isReadable() const { return 0 < readable(); }

  /** Determine whether there is space to write additional characters.
   *
   *
   * @return whether there are remaining characters/space
   */
  inline bool isWritable() const { return 0 < writable(); }

  /** Write a character to the buffer and advance the write position.  If the
   * buffer is full this is a no-op.
   *
   * @param character The character to be added to the buffer.
   */
  inline void putNext(unsigned char character) {
    if (!isWritable()) {
      return;
    }

    _buffer[_writePosition++] = character;
  }

  /** Read a character from the buffer and advance the read position.  If the
   * buffer is empty this is a no-op.
   *
   * @return The next character or 0 if there are no more characters
   */
  inline unsigned char next() { return isReadable() ? _buffer[_readPosition++] : 0; }

  /** Read the next character in the buffer but do not advance the read
   * position.  If the buffer is empty this is a no-op.
   *
   * @return The next character or 0 if there are no more characters.
   */
  inline unsigned char peekNext() const { return isReadable() ? _buffer[_readPosition] : 0; }

  /** Skip the next character in the buffer.  If
   *  the buffer is empty this is a no-op.  Buffer should be in drain mode.
   */
  inline void skipNext() { _readPosition = !isReadable() ? _readPosition : _readPosition + 1; }

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
  inline UInt32 writePosition() const { return _writePosition; }

  /** Get the current read position.
   *
   * @return The current read position
   */
  inline UInt32 readPosition() const { return _readPosition; }

  /** Set the current write position
   *
   * @param position The new value for the write position
   */
  inline void setWritePosition(UInt32 position) { _writePosition = position > _capacity ? _capacity : position; }

  /** Set the current read position
   *
   * @param position The new value for the read position
   */
  inline void setReadPosition(UInt32 position) { _readPosition = position > _capacity ? _capacity : position; }

  /** Get the underlying buffer.
   *
   * @return The underlying buffer
   */
  inline unsigned char *buffer() { return _buffer; }

  /** Get the underlying buffer.
   *
   * @return The underlying buffer
   */
  inline const unsigned char *buffer() const { return _buffer; }

  /** Advance the read position n steps up to the write position, but no more.
   *
   * @param n The number of steps to advance the read position
   */
  inline void skip(UInt32 n) {
    _readPosition = _readPosition + n > _writePosition ? _writePosition : _readPosition + n;
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
  unsigned char *duplicate(Allocator &allocator, bool trim) const;

  /** Duplicate the data in the buffer.  This will duplicate everything from the
   * 0th position to the write position.  The duplicate will be null terminated
   * and any trailing whitespace (' ', '\t', '\r', '\n') will be trimmed.
   *
   * @param allocator The memory for the duplicate will be taken from this
   * allocator
   * @return The duplicate or NULL if memory could not be allocated.
   */
  inline unsigned char *duplicate(Allocator &allocator) const { return duplicate(allocator, true); }

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
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

  /** Placement new
   *
   * @param size The size of the memory block
   * @param block A valid memory block with which the object can be constructed.
   * @return The memory block
   */
  inline void *operator new(size_t size, unsigned char *block) noexcept { return block; }

 private:
  // Disabled
  Buffer(const Buffer &);
  Buffer &operator=(const Buffer &);

  UInt32 _readMark;
  UInt32 _writeMark;
  UInt32 _readPosition;
  UInt32 _writePosition;
  UInt32 _capacity;
  unsigned char *_buffer;
};

}  // namespace ESB

#endif
