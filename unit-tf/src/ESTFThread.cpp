#ifndef ESTF_THREAD_H
#include "ESTFThread.h"
#endif

#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

namespace ESTF {

void *Thread::ThreadEntry( void * arg )
{
    if ( ! arg ) return 0;

    ( ( Thread * ) arg )->run();

    return 0;
}

/** Default constructor. */
Thread::Thread()
{
}

/** Default destructor. */
Thread::~Thread()
{
}

bool Thread::spawn()
{
#ifdef HAVE_PTHREAD_CREATE
    return 0 == pthread_create( &_threadId, 0, &ThreadEntry, this );
#else
#error "pthread_create or equivalent is required"
#endif
}

bool Thread::join()
{
#ifdef HAVE_PTHREAD_JOIN
    return 0 == pthread_join( _threadId, 0 );
#else
#error "pthread_join or equivalent is required"
#endif
}

Thread::ThreadId Thread::getThreadId()
{
    return _threadId;
}

void
Thread::Yield()
{
#ifdef HAVE_PTHREAD_YIELD
    pthread_yield();
#elif defined HAVE_SCHED_YIELD
    sched_yield();
#else
#error "pthread_yield or equivalent is required"
#endif
}


Thread::ThreadId Thread::GetThreadId()
{
#ifdef HAVE_PTHREAD_SELF
    return pthread_self();
#else
#error "pthread_self or equivalent is required"
#endif
}

void
Thread::Sleep( long msec )
{
#if defined HAVE_NANOSLEEP && defined HAVE_TIMESPEC_T
    struct timespec timeToSleep;

    timeToSleep.tv_sec = msec / 1000;
    timeToSleep.tv_nsec = ( msec % 1000 ) * 1000L * 1000L;

    nanosleep( &timeToSleep, 0 );
#else
#error "nanosleep or equivalent is required"
#endif
}

}
