/** @file ESFBufferPool.h
 *  @brief A synchronized pool of ESFBuffers
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_BUFFER_POOL_H
#define ESF_BUFFER_POOL_H

#ifndef ESF_BUFFER_H
#include <ESFBuffer.h>
#endif

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

#ifndef ESF_WRITE_SCOPE_LOCK_H
#include <ESFWriteScopeLock.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_EMBEDDED_LIST_H
#include <ESFEmbeddedList.h>
#endif

/** @defgroup util Utilities */

/** A synchronized pool of ESFBuffers
 *
 *  @ingroup util
 */
class ESFBufferPool {
public:

    /** Constructor.
     *
     * @param numberOfBuffers The maximum number of buffers to keep in the pool
     * @param bufferSize The size in bytes of each individual buffer
     * @param allocator The allocator to use to allocate buffers.
     */
    ESFBufferPool(int numberOfBuffers, int bufferSize, ESFAllocator *allocator);

    /** Destructor.
     */
    virtual ~ESFBufferPool();

    /** Get a buffer from the buffer pool.  If there are no buffers in the buffer
     *  pool, allocate a new buffer.
     *
     * @return A buffer or NULL if there were no buffers in the buffer pool and
     *  a new buffer could not be allocated.
     */
    ESFBuffer *acquireBuffer();

    /** Return a buffer to the buffer pool.  If the buffer pool is full (i.e.,
     *  there <code>numberOfBuffers</code> in the pool), the buffer will be
     *  freed.
     *
     * @param buffer The buffer to return to the pool
     */
    void releaseBuffer(ESFBuffer *buffer);

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return Memory for the new object or NULL if the memory allocation failed.
     */
    inline void *operator new(size_t size, ESFAllocator *allocator) {
        return allocator->allocate(size);
    }

private:
    // Disabled
    ESFBufferPool(const ESFBufferPool &);
    ESFBufferPool &operator=(const ESFBufferPool &);

    int _numberOfBuffers;
    int _bufferSize;
    int _listSize;
    ESFAllocator *_allocator;
    ESFEmbeddedList _embeddedList;
    ESFMutex _lock;
};

#endif /* ! ESF_BUFFER_POOL_H */
