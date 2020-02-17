#ifndef ESB_BUFFER_POOL_H
#define ESB_BUFFER_POOL_H

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

namespace ESB {

/** @defgroup util Utilities */

/** A synchronized pool of Buffers
 *
 *  @ingroup util
 */
class BufferPool {
 public:
  /** Constructor.
   *
   * @param numberOfBuffers The maximum number of buffers to keep in the pool
   * @param bufferSize The size in bytes of each individual buffer
   * @param allocator The allocator to use to allocate buffers.
   */
  BufferPool(int numberOfBuffers, int bufferSize, Allocator *allocator);

  /** Destructor.
   */
  virtual ~BufferPool();

  /** Get a buffer from the buffer pool.  If there are no buffers in the buffer
   *  pool, allocate a new buffer.
   *
   * @return A buffer or NULL if there were no buffers in the buffer pool and
   *  a new buffer could not be allocated.
   */
  Buffer *acquireBuffer();

  /** Return a buffer to the buffer pool.  If the buffer pool is full (i.e.,
   *  there <code>numberOfBuffers</code> in the pool), the buffer will be
   *  freed.
   *
   * @param buffer The buffer to return to the pool
   */
  void releaseBuffer(Buffer *buffer);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  // Disabled
  BufferPool(const BufferPool &);
  BufferPool &operator=(const BufferPool &);

  int _numberOfBuffers;
  int _bufferSize;
  int _listSize;
  Allocator *_allocator;
  EmbeddedList _embeddedList;
  Mutex _lock;
};

}  // namespace ESB

#endif
