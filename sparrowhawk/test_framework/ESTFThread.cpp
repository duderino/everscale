/** @file ESTFThread.cpp
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
#include "ESTFThread.h"
#endif

#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

void *ESTFThread::ThreadEntry( void * arg )
{
    if ( ! arg ) return 0;

    ( ( ESTFThread * ) arg )->run();

    return 0;
}

/** Default constructor. */
ESTFThread::ESTFThread()
{
}

/** Default destructor. */
ESTFThread::~ESTFThread()
{
}

bool ESTFThread::spawn()
{
#ifdef HAVE_PTHREAD_CREATE
    return 0 == pthread_create( &_threadId, 0, &ThreadEntry, this );
#else
#error "pthread_create or equivalent is required"
#endif
}

bool ESTFThread::join()
{
#ifdef HAVE_PTHREAD_JOIN
    return 0 == pthread_join( _threadId, 0 );
#else
#error "pthread_join or equivalent is required"
#endif
}

ESTFThread::ThreadId ESTFThread::getThreadId()
{
    return _threadId;
}

void
ESTFThread::Yield()
{
#ifdef HAVE_PTHREAD_YIELD
    pthread_yield();
#elif defined HAVE_SCHED_YIELD
    sched_yield();
#else
#error "pthread_yield or equivalent is required"
#endif
}


ESTFThread::ThreadId ESTFThread::GetThreadId()
{
#ifdef HAVE_PTHREAD_SELF
    return pthread_self();
#else
#error "pthread_self or equivalent is required"
#endif
}

void
ESTFThread::Sleep( long msec )
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
