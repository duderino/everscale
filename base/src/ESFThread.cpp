/** @file ESFThread.cpp
 *  @brief An abstract class that any class can subclass to run in its own
 *      thread of control
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:08 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESF_THREAD_H
#include "ESFThread.h"
#endif

#if !defined HAVE_PTHREAD_YIELD && defined HAVE_SCHED_H && \
    defined HAVE_SCHED_YIELD
#include <sched.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

void *ESFThread::ThreadEntry(void *arg) {
  if (!arg) return 0;

  ((ESFThread *)arg)->run();

  return 0;
}

ESFThread::ESFThread() : _isRunning(true) {}

ESFThread::~ESFThread() {}

ESFError ESFThread::start() {
#ifdef HAVE_PTHREAD_CREATE
  return ESFConvertError(
      pthread_create(&_threadId, 0, &ThreadEntry, (ESFThread *)this));
#else
#error "pthread_create or equivalent is required"
#endif
}

ESFError ESFThread::join() {
#ifdef HAVE_PTHREAD_JOIN
  return ESFConvertError(pthread_join(_threadId, 0));
#else
#error "pthread_join or equivalent is required"
#endif
}

ESFThread::ThreadId ESFThread::getThreadId() const { return _threadId; }

void ESFThread::Yield() {
#ifdef HAVE_PTHREAD_YIELD
  pthread_yield();
#elif defined HAVE_SCHED_YIELD
  sched_yield();
#else
#error "pthread_yield or equivalent is required"
#endif
}

void ESFThread::Sleep(const ESFDate &date) {
#if defined HAVE_NANOSLEEP && defined HAVE_TIMESPEC_T
  struct timespec timeToSleep;

  timeToSleep.tv_sec = date.getSeconds();
  timeToSleep.tv_nsec = date.getMicroSeconds() * 1000L;

  nanosleep(&timeToSleep, 0);
#else
#error "nanosleep or equivalent is required"
#endif
}

void ESFThread::Sleep(long msec) {
#if defined HAVE_NANOSLEEP && defined HAVE_TIMESPEC_T
  struct timespec timeToSleep;

  timeToSleep.tv_sec = msec / 1000;
  timeToSleep.tv_nsec = (msec % 1000) * 1000L * 1000L;

  nanosleep(&timeToSleep, 0);
#else
#error "nanosleep or equivalent is required"
#endif
}

ESFThread::ThreadId ESFThread::GetThreadId() {
#ifdef HAVE_PTHREAD_SELF
  return pthread_self();
#else
#error "pthread_self or equivalent is required"
#endif
}
