/** @file ESFThreadPool.h
 *  @brief A thread pool that executes ESFCommands
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

#ifndef ESF_THREAD_POOL_H
#define ESF_THREAD_POOL_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ERROR_H
#include <ESFError.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

#ifndef ESF_THREAD_H
#include <ESFThread.h>
#endif

#ifndef ESF_SHARED_EMBEDDED_QUEUE_H
#include <ESFSharedEmbeddedQueue.h>
#endif

#ifndef ESF_COMMAND_H
#include <ESFCommand.h>
#endif

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

/** A thread pool that executes ESFCommands
 *
 *  @ingroup thread
 */
class ESFThreadPool {
public:

    /** Constructor.
     *
     * @param name The name of the thread pool (included in all log messages).
     *  Caller responsible for the strings memory - use a string literal if possible.
     * @param threads The number of threads to start for the thread pool.
     * @param logger An optional logger.  Pass NULL to not log anything.
     * @param allocator Worker threads will be allocated with this
     *  allocator.
     */
    ESFThreadPool(const char *name, int threads, ESFLogger *logger, ESFAllocator *allocator);

    /** Destructor.
     */
    virtual ~ESFThreadPool();

    /** Start the threads in the thread pool
     *
     * @return ESF_SUCCESS if successful, another error code otherwise.
     */
    ESFError start();

    /** Stop the thread pool immediately.  Each queued command that is
     *  not executed will be passed to a cleanup handler.  Function
     *  will return after all worker threads have exited.
     */
    void stop();

    /** Run a command in the thread pool.  If the thread pool has
     *  not been started, the commands will just sit in an internal
     *  queue until it is started.
     *
     * @param command The command to execute
     * @return ESF_SUCCESS if successful, ESF_SHUTDOWN if stop has
     *  already been called, another error code otherwise.
     */
    inline ESFError execute(ESFCommand *command) {
        return _queue.push(command);
    }

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return The new object or NULL of the memory allocation failed.
     */
    inline void *operator new(size_t size, ESFAllocator *allocator) {
        return allocator->allocate(size);
    }

    /** Get the size in bytes used for each worker thread.  This
     *  might help create an appropriate allocator for the
     *  thread pool, or the default system allocator could just be
     *  used.
     *
     * @return The size in bytes for each worker thread.
     */
    static ESFUWord GetWorkerThreadSize();

private:

    //  Disabled
    ESFThreadPool(const ESFThreadPool &);
    ESFThreadPool &operator=(const ESFThreadPool &);

    bool createWorkerThreads();
    void destroyWorkerThreads();

    int _numThreads;
    const char *_name;
    ESFThread **_threads;
    ESFAllocator *_allocator;
    ESFLogger *_logger;

    ESFSharedEmbeddedQueue _queue;
};

#endif /* ! ESF_THREAD_POOL_H */
