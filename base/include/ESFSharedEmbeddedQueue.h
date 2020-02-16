/** @file ESFSharedEmbeddedQueue.h
 *  @brief A producer/consumer queue of ESFEmbeddedListElements
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

#ifndef ESF_SHARED_EMBEDDED_QUEUE_H
#define ESF_SHARED_EMBEDDED_QUEUE_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ERROR_H
#include <ESFError.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

#ifndef ESF_EMBEDDED_LIST_H
#include <ESFEmbeddedList.h>
#endif

#ifndef ESF_EMBEDDED_LIST_ELEMENT_H
#include <ESFEmbeddedListElement.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifndef ESF_COUNTING_SEMAPHORE_H
#include <ESFCountingSemaphore.h>
#endif

/** A producer/consumer queue of ESFEmbeddedListElements
 *
 *  @ingroup collection
 */
class ESFSharedEmbeddedQueue {
public:

    /** Constructor.
     *
     * @param cleanupHandler Any items in the queue when it is stopped will
     *  be passsed to this handler for destruction.
     */
    ESFSharedEmbeddedQueue();

    /** Destructor.
     */
    virtual ~ESFSharedEmbeddedQueue();

    /** Push an element into the queue
     *
     * @param element The element to insert
     * @return ESF_SUCCESS if successful, another error code otherwise.
     */
    ESFError push(ESFEmbeddedListElement *element);

    /** Pop an element from the queue, blocking until an element
     *  is available if necessary.
     *
     * @param error An optional ESFError to receive the result.  Will be
     *  set to ESF_SUCCESS if successful, ESF_SHUTDOWN if the queue has been
     *  stopped, another error code otherwise.
     * @return An element or NULL if the operation failed.
     */
    ESFEmbeddedListElement *pop(ESFError *error = 0);

    /** Stop the queue.  This will do three things.  New pushes will
     *  fail with the error ESF_SHUTDOWN.  Threads blocked on pop calls will
     *  immediately return with ESF_SHUTDOWN.  Items in the queue will be
     *  passed to the cleanup handler and removed.
     */
    void stop();

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return The memory for the new object or NULL of the memory allocation failed.
     */
    inline void *operator new(size_t size, ESFAllocator *allocator) {
        return allocator->allocate(size);
    }

private:

    //  Disabled
    ESFSharedEmbeddedQueue(const ESFSharedEmbeddedQueue &);
    ESFSharedEmbeddedQueue &operator=(const ESFSharedEmbeddedQueue &);

    bool _isStopped;

#if defined HAVE_PTHREAD_MUTEX_T && defined HAVE_PTHREAD_COND_T
    pthread_mutex_t _mutex;
    pthread_cond_t _signal;
#else
#error "Mutex and condition variable or equivalent is required"
#endif

    ESFEmbeddedList _list;
};

#endif /* ! ESF_SHARED_EMBEDDED_QUEUE_H */
