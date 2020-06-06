#ifndef ESB_BUFFER_POOL_H
#define ESB_BUFFER_POOL_H

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_NULL_LOCK_H
#include <ESBNullLock.h>
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
   * @param bufferSize The size in bytes of each individual buffer
   * @param maxBuffers The maximum number of buffers to keep in the pool. 0 for
   * infinite.  Defaults to infinite.
   * @param lock A lock to use for internal synchronization.  Defaults to
   * null/no locking.
   * @param allocator The allocator to use to allocate buffers.  Defaults to
   * malloc.
   */
  BufferPool(UInt32 bufferSize, UInt32 maxBuffers = 0, Lockable &lock = NullLock::Instance(),
             Allocator &allocator = SystemAllocator::Instance());

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
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  // Disabled
  BufferPool(const BufferPool &);
  BufferPool &operator=(const BufferPool &);

  const UInt32 _maxBuffers;
  const UInt32 _bufferSize;
  UInt32 _listSize;
  Lockable &_lock;
  Allocator &_allocator;
  EmbeddedList _embeddedList;
};

}  // namespace ESB

#endif
