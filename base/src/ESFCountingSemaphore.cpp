/** @file ESFCountingSemaphore.cpp
 *  @brief An counting semaphore wrapper that also implements the ESFLockable
 *  interface.
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

#ifndef ESF_COUNTING_SEMAPHORE_H
#include <ESFCountingSemaphore.h>
#endif

ESFCountingSemaphore::ESFCountingSemaphore() : _magic(0) {
#if defined HAVE_SEM_INIT && 2147483647 == SEM_VALUE_MAX

  if (0 == sem_init(&_semaphore, 0, 0)) {
    _magic = ESF_MAGIC;
  }

#elif defined HAVE_PTHREAD_MUTEX_INIT && defined HAVE_PTHREAD_COND_INIT && \
    defined HAVE_PTHREAD_MUTEX_DESTROY

  _semaphore._count = 0;

  if (0 != pthread_mutex_init(&_semaphore._mutex, 0)) {
    return;
  }

  if (0 != pthread_cond_init(&_semaphore._cond, 0)) {
    pthread_mutex_destroy(&_semaphore._mutex);
    return;
  }

#elif defined HAVE_CREATE_SEMAPHORE

  _semaphore = CreateSemaphore(0, 0, 2147483647, 0);

  if (!_semaphore) {
    return;
  }

#else
#error "Platform has no counting semaphore initializer"
#endif

  _magic = ESF_MAGIC;
}

ESFCountingSemaphore::~ESFCountingSemaphore() {
  if (ESF_MAGIC != _magic) {
    return;
  }

#if defined HAVE_SEM_DESTROY && 2147483647 == SEM_VALUE_MAX

  sem_destroy(&_semaphore);

#elif defined HAVE_PTHREAD_MUTEX_DESTROY && defined HAVE_PTHREAD_COND_DESTROY

  pthread_mutex_destroy(&_semaphore._mutex);
  pthread_cond_destroy(&_semaphore._cond);

#elif defined HAVE_CLOSE_HANDLE

  CloseHandle(_semaphore);

#else
#error "Platform has no counting semaphore destructor."
#endif
}

ESFError ESFCountingSemaphore::writeAcquire() {
  if (ESF_MAGIC != _magic) {
    return ESF_NOT_INITIALIZED;
  }

#if defined HAVE_SEM_WAIT && 2147483647 == SEM_VALUE_MAX

  ESFError error;

  while (true) {
    if (0 == sem_wait(&_semaphore)) {
      return ESF_SUCCESS;
    }

    error = ESFGetLastError();

    if (ESF_INTR == error) {
      continue;
    }

    return error;
  }

  return ESF_SUCCESS;

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_COND_WAIT && \
    defined HAVE_PTHREAD_MUTEX_UNLOCK

  ESFError error;

  error = ESFConvertError(pthread_mutex_lock(&_semaphore._mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  while (1 > _semaphore._count) {
    error = ESFConvertError(
        pthread_cond_wait(&_semaphore._cond, &_semaphore._mutex));

    if (ESF_SUCCESS != error && ESF_INTR != error) {
      return error;
    }
  }

  --_semaphore._count;

  return ESFConvertError(pthread_mutex_unlock(&_semaphore._mutex));

#elif defined HAVE_WAIT_FOR_SINGLE_OBJECT

  switch (WaitForSingleObject(_semaphore, INFINITE)) {
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
#error "Platform has no counting semaphore write lock function."
#endif
}

ESFError ESFCountingSemaphore::readAcquire() { return writeAcquire(); }

ESFError ESFCountingSemaphore::writeAttempt() {
  if (ESF_MAGIC != _magic) {
    return ESF_NOT_INITIALIZED;
  }

#if defined HAVE_SEM_TRYWAIT && 2147483647 == SEM_VALUE_MAX

  if (0 == sem_trywait(&_semaphore)) {
    return ESF_SUCCESS;
  }

  // ESFGetLastError will map EAGAIN to ESF_AGAIN
  return ESFGetLastError();

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK && \
    defined HAVE_PTHREAD_MUTEX_UNLOCK

  ESFError error;

  error = ESFConvertError(pthread_mutex_lock(&_semaphore._mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  if (0 < _semaphore._count) {
    --_semaphore._count;

    return ESFConvertError(pthread_mutex_unlock(&_semaphore._mutex));
  }

  error = pthread_mutex_unlock(&_semaphore._mutex);

  return ESF_SUCCESS == error ? ESF_AGAIN : error;

#elif defined HAVE_WAIT_FOR_SINGLE_OBJECT

  switch (WaitForSingleObject(_semaphore, 0)) {
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
#error "Platform has no counting semaphore write trylock function."
#endif
}

ESFError ESFCountingSemaphore::readAttempt() { return writeAttempt(); }

ESFError ESFCountingSemaphore::writeRelease() {
  if (ESF_MAGIC != _magic) {
    return ESF_NOT_INITIALIZED;
  }

#if defined HAVE_SEM_POST && 2147483647 == SEM_VALUE_MAX

  if (0 == sem_post(&_semaphore)) {
    return ESF_SUCCESS;
  }

  return ESFGetLastError();

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK && \
    defined HAVE_PTHREAD_COND_SIGNAL

  ESFError error;

  error = ESFConvertError(pthread_mutex_lock(&_semaphore._mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  if (ESF_INT32_MAX == _semaphore._count) {
    pthread_mutex_unlock(&_semaphore._mutex);

    return ESF_OVERFLOW;
  }

  ++_semaphore._count;

  error = ESFConvertError(pthread_cond_signal(&_semaphore._cond));

  if (ESF_SUCCESS != error) {
    pthread_mutex_unlock(&_semaphore._mutex);

    return error;
  }

  return ESFConvertError(pthread_mutex_unlock(&_semaphore._mutex));

#elif defined HAVE_RELEASE_SEMAPHORE

  if (0 != ReleaseSemaphore(_semaphore, 1, 0)) {
    return ESF_SUCCESS;
  }

  return ESFGetLastError();

#else
#error "Platform has no counting semaphore write unlock function."
#endif
}

ESFError ESFCountingSemaphore::readRelease() { return writeRelease(); }
