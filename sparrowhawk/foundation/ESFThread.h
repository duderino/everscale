/** @file ESFThread.h
 *  @brief An abstract class that any class can subclass to run in its own
 *      thread of control
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:08 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESF_THREAD_H
#define ESF_THREAD_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

#ifndef ESF_DATE_H
#include <ESFDate.h>
#endif

#ifndef ESF_FLAG_H
#include <ESFFlag.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#else
#error "pthread.h or equivalent is required"
#endif

/** @defgroup thread Thread */

/** ESFThread should be subclassed by any class that wants to run in its own
 *  thread of control.  Subclasses must define a run template method that this
 *  base class will call in a new thread of control.
 *
 *  @ingroup thread
 */
class ESFThread {
public:
#ifdef HAVE_PTHREAD_T
    typedef pthread_t ThreadId;
#else
#error "pthread_t or equivalent is required"
#endif

    /** Default constructor.
     */
    ESFThread();

    /** Default destructor.
     */
    virtual ~ESFThread();

    /** Spawns a new thread that will immediately call the run method.
     *
     *  @return ESF_SUCCESS if new thread was spawned, another error code otherwise.
     */
    ESFError start();

    /** Requests the thread stop.  Threads may choose to ignore this request.
     *
     */
    inline void stop() {
        _isRunning.set(false);
    }

    /** Blocks the calling thread until this thread finishes execution.
     *
     *  @return ESF_SUCCESS if join was successful, another error code otherwise.
     */
    ESFError join();

    /** Get the thread id of this thread.
     *
     *    @return the thread id of this thread.
     */
    ThreadId getThreadId() const;

    /** Get the current thread id of the calling thread.
     *
     *    @return the thread id of the calling thread.
     */
    static ThreadId GetThreadId();

    /** Yield the processor to another thread or process.
     */
    static void Yield();

    /** Put the calling thread to sleep
     *
     *  @param date The amount of time to sleep
     */
    static void Sleep(const ESFDate &date);

    /** Put the calling thread to sleep
     *
     *  @param msec The number of milliseconds to sleep
     */
    static void Sleep(long msec);

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return The new object or NULL of the memory allocation failed.
     */
    inline void *operator new(size_t size, ESFAllocator *allocator) {
        return allocator->allocate(size);
    }

protected:

    /** This is the main function for the new thread.  Subclasses must define
     *    this.
     */
    virtual void run() = 0;

    /** Should be called from a subclass's run method on every iteration to
     *  determine whether it should return from its run method.
     *
     *  @return true if the thread should continue running, false otherwise.
     */
    inline bool isRunning() const {
        return _isRunning.get();
    }

    ESFFlag _isRunning;

private:

    //  Disabled
    ESFThread(const ESFThread &);
    ESFThread &operator=(const ESFThread &);

    static void *ThreadEntry(void *arg);

    ThreadId _threadId;
};

#endif /* ! ESF_THREAD_H */
