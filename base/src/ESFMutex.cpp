/** @file ESFMutex.cpp
 *  @brief A simple mutual exclusion implementation of the ESFLockable interface
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

ESFMutex::ESFMutex() : _magic(0) {
#ifdef HAVE_PTHREAD_MUTEX_INIT

  int error = pthread_mutex_init(&_mutex, 0);

  ESF_ASSERT(0 == error);

  if (0 == error) {
    _magic = ESF_MAGIC;
  }

#elif defined HAVE_CREATEMUTEX

  _mutex = CreateMutex(0, false, 0);

  if (_mutex) {
    _magic = ESF_MAGIC;
  }

#else
#error "Platform has no mutex initializer"
#endif
}

ESFMutex::~ESFMutex() {
  if (ESF_MAGIC != _magic) {
    return;
  }

#ifdef HAVE_PTHREAD_MUTEX_DESTROY

  pthread_mutex_destroy(&_mutex);

#elif defined HAVE_CLOSE_HANDLE

  CloseHandle(_mutex);

#else
#error "Platform has no mutex destructor."
#endif
}

ESFError ESFMutex::writeAcquire() {
  if (ESF_MAGIC != _magic) {
    return ESF_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_MUTEX_LOCK

  return ESFConvertError(pthread_mutex_lock(&_mutex));

#elif defined HAVE_WAIT_FOR_SINGLE_OBJECT

  switch (WaitForSingleObject(_mutex, INFINITE)) {
    case WAIT_OBJECT_0:

      return ESF_SUCCESS;

    case WAIT_TIMEOUT:

      return ESF_AGAIN;

    case WAIT_FAILED:

      return ESFGetLastError();

    default:

      return ESF_OTHER_ERROR;
  }

#else
#error "Platform has no mutex lock function."
#endif
}

ESFError ESFMutex::readAcquire() { return writeAcquire(); }

ESFError ESFMutex::writeAttempt() {
  if (ESF_MAGIC != _magic) {
    return ESF_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_MUTEX_TRYLOCK

  int error = pthread_mutex_trylock(&_mutex);

  switch (error) {
    case 0:
      return ESF_SUCCESS;

    case EBUSY:
      return ESF_AGAIN;

    default:
      return ESFConvertError(error);
  }

#elif defined HAVE_WAIT_FOR_SINGLE_OBJECT

  switch (WaitForSingleObject(_mutex, 0)) {
    case WAIT_OBJECT_0:

      return ESF_SUCCESS;

    case WAIT_TIMEOUT:

      return ESF_AGAIN;

    case WAIT_FAILED:

      return ESFGetLastError();

    default:

      return ESF_OTHER_ERROR;
  }

#else
#error "Platform has no mutex trylock function."
#endif
}

ESFError ESFMutex::readAttempt() { return writeAttempt(); }

ESFError ESFMutex::writeRelease() {
  if (ESF_MAGIC != _magic) {
    return ESF_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_MUTEX_UNLOCK

  return ESFConvertError(pthread_mutex_unlock(&_mutex));

#elif defined HAVE_RELEASEMUTEX

  if (0 == ReleaseMutex(_mutex)) {
    return ESFGetLastError();
  }

  return ESF_SUCCESS;

#else
#error "Platform has no mutex unlock function."
#endif
}

ESFError ESFMutex::readRelease() { return writeRelease(); }
