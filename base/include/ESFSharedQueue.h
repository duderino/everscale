/** @file ESFSharedQueue.h
 *  @brief A threadsafe queue good for producer/consumer problems
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

#ifndef ESF_SHARED_QUEUE_H
#define ESF_SHARED_QUEUE_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ERROR_H
#include <ESFError.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

#ifndef ESF_LIST_H
#include <ESFList.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

/** ESFSharedQueue is a self-synchronizing queue that handles many producer
 *  threads and many consumer threads.  Consumer threads can block until an
 *  item is added to the queue or return immediately.  Producer threads can
 *  block until there is room in the queue to add an item or return
 *  immediately.
 *
 *  @ingroup collection
 */
class ESFSharedQueue {
public:

    /** Constructor.
     *
     *  @param allocator The allocator that the queue will use to create its
     *      internal nodes.
     *  @param limit The maximum number of items that can be inserted into the
     *      queue.  Set to 0 for unlimited.  Queues that have no limit will
     *      grow as long as their allocator continue to return memory.
     *  @see GetAllocationSize to determine how much memory the list will
     *      allocate for every internal node it creates.  This is useful for
     *      constructing fixed length allocators.
     */
    ESFSharedQueue(ESFAllocator *allocator, ESFUInt32 limit);

    /** Destructor. */
    virtual ~ESFSharedQueue();

    /** Insert an item into the queue.  O(1).  Block the caller until there
     *  is sufficient room to add a new item.
     *
     *  @param item The item to insert.  Cannot be NULL.
     *  @return ESF_SUCCESS if successful, another error code otherwise.
     */
    ESFError push(void *item);

    /** Insert an item into the queue.  O(1).  Return immediately if there
     *  is not sufficient room to add a new item.
     *
     *  @param item The item to insert.  Cannot be NULL.
     *  @return ESF_SUCCESS if successful, ESF_AGAIN if there is insufficient
     *      room, another error code otherwise.
     */
    ESFError tryPush(void *item);

    /** Retrieve an item from the queue.  O(1).  Block the caller until an item
     *  is added to queue.
     *
     *  @param item An item from the queue.
     *  @return ESF_SUCCESS if successful, another error code otherwise.
     */
    ESFError pop(void **item);

    /** Retrieve an item from the queue.  O(1).  Return immediately if an item
     *  is not available.
     *
     *  @param item An item from the queue
     *  @return ESF_SUCCES if successful, ESF_AGAIN if no items are available,
     *      another error code otherwise.
     */
    ESFError tryPop(void **item);

    /** Remove all items from the queue.  O(n).  This will only deallocate
     *  memory used by the list's internal nodes.
     *
     *  @return ESF_SUCCESS if successful, another error code otherwise.
     */
    ESFError clear();

    /** Get the current size of the queue.  O(1).
     *
     *  @param size The current size of the queue
     *  @return ESF_SUCCESS if successful, another error code otherwise
     */
    ESFError getSize(ESFUInt32 *size);

    /** Get the size in bytes of the queue's interal nodes.  This is the amount
     *  of memory that the queue will request from the allocator for every node
     *  it creates.
     *
     *  @return The size in bytes of the queue's internal nodes.
     */
    static ESFSize GetAllocationSize();

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return The new object or NULL of the memory allocation failed.
     */
    inline void *operator new(size_t size, ESFAllocator *allocator) {
        return allocator->allocate(size);
    }

private:

    //  Disabled
    ESFSharedQueue(const ESFSharedQueue &);
    ESFSharedQueue &operator=(const ESFSharedQueue &);

#if defined HAVE_PTHREAD_MUTEX_T && defined HAVE_PTHREAD_COND_T
    class Synchronizer {
    public:
        /** Default constructor */
        Synchronizer();

        /** Destructor */
        ~Synchronizer();

        pthread_mutex_t _mutex;
        pthread_cond_t _consumerSignal;
        pthread_cond_t _producerSignal;
        ESFUInt8 _magic;
    };
#else
#error "Mutex and condition variable or equivalent is required"
#endif

    ESFUInt32 _limit;
    Synchronizer _lock;
    ESFList _list;
};

#endif /* ! ESF_SHARED_QUEUE_H */
