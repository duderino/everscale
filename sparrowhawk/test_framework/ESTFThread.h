/** @file ESTFThread.h
 *  @brief A platform-independent thread abstraction
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:19 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESTF_THREAD_H
#define ESTF_THREAD_H

#ifndef ESTF_CONFIG_H
#include <ESTFConfig.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

/** ESTFThread should be subclassed by any class that wants to run in its own
 *  thread of control.  Subclasses must define a run template method that this
 *  base class will call in a new thread of control.
 *
 *  @ingroup test
 */
class ESTFThread
{
public:
#ifdef HAVE_PTHREAD_T
    typedef pthread_t ThreadId;
#else
#error "thread type required"
#endif

    /** Default constructor. */
    ESTFThread();

    /** Default destructor. */
    virtual ~ESTFThread();

    /** Spawns a new thread that will immediately call the run method.
     *
     *    @return true if new thread was spawned, false otherwise.
     */
    bool spawn();

    /** Blocks the calling thread until this thread finishes execution.
     *
     *    @return true if join was successful, false otherwise.
     */
    bool join();

    /** Get the thread id of this thread.
     *
      *    @return the thread id of this thread.
     */
    ThreadId getThreadId();

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
     *  @param msec The number of milliseconds to sleep
     */
    static void Sleep( long msec );

protected:

    /** This is the main function for the new thread.  Subclasses must define
     *    this.
     */
    virtual void run() = 0;

private:

    static void *ThreadEntry( void * arg );

    ThreadId _threadId;
};

#endif /* ! ESTF_THREAD_H */
