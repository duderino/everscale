/** @file ESFReadWriteLock.cpp
 *  @brief A multiple readers, single writer implementation of the ESFLockable
 *      interface
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

#ifndef ESF_READ_WRITE_LOCK_H
#include <ESFReadWriteLock.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

ESFReadWriteLock::ESFReadWriteLock() : _magic(0) {
#ifdef HAVE_PTHREAD_RWLOCK_INIT

  if (0 == pthread_rwlock_init(&_lock, 0)) {
    _magic = ESF_MAGIC;
  }

#elif defined HAVE_PTHREAD_MUTEX_INIT && defined HAVE_PTHREAD_COND_INIT && \
    defined HAVE_PTHREAD_MUTEX_DESTROY && defined HAVE_PTHREAD_COND_DESTROY

  if (0 != pthread_mutex_init(&_lock._mutex, 0)) {
    return;
  }

  if (0 != pthread_cond_init(&_lock._readSignal, 0)) {
    pthread_mutex_destroy(&_lock._mutex);
    return;
  }

  if (0 != pthread_cond_init(&_lock._writeSignal, 0)) {
    pthread_mutex_destroy(&_lock._mutex);
    pthread_cond_destroy(&_lock._readSignal);
    return;
  }

  _lock._readersActive = 0;
  _lock._readersWaiting = 0;
  _lock._writersActive = 0;
  _lock._writersWaiting = 0;

  _magic = ESF_MAGIC;

#else
#error "Platform has no rw lock initializer"
#endif
}

ESFReadWriteLock::~ESFReadWriteLock() {
  if (ESF_MAGIC != _magic) {
    return;
  }

#ifdef HAVE_PTHREAD_RWLOCK_DESTROY

  pthread_rwlock_destroy(&_lock);

#elif defined HAVE_PTHREAD_MUTEX_DESTROY && defined HAVE_PTHREAD_COND_DESTROY

  pthread_mutex_destroy(&_lock._mutex);
  pthread_cond_destroy(&_lock._readSignal);
  pthread_cond_destroy(&_lock._writeSignal);

#else
#error "Platform has no rw lock destructor."
#endif
}

ESFError ESFReadWriteLock::writeAcquire() {
  if (ESF_MAGIC != _magic) {
    return ESF_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_RWLOCK_WRLOCK

  return ESFConvertError(pthread_rwlock_wrlock(&_lock));

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_COND_WAIT && \
    defined HAVE_PTHREAD_MUTEX_UNLOCK

  ESFError error = ESFConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  while (0 < _lock._writersActive || 0 < _lock._readersActive) {
    ++_lock._writersWaiting;

    error =
        ESFConvertError(pthread_cond_wait(&_lock._writeSignal, &_lock._mutex));

    --_lock._writersWaiting;

    if (ESF_SUCCESS != error) {
      pthread_mutex_unlock(&_lock._mutex);

      return error;
    }
  }

  ESF_ASSERT(0 == _lock._writersActive && 0 == _lock._readersActive);

  _lock._writersActive = 1;

  return ESFConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no rw lock write lock function."
#endif
}

ESFError ESFReadWriteLock::readAcquire() {
  if (ESF_MAGIC != _magic) {
    return ESF_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_RWLOCK_RDLOCK

  return ESFConvertError(pthread_rwlock_rdlock(&_lock));

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_COND_WAIT && \
    defined HAVE_PTHREAD_MUTEX_UNLOCK

  ESFError error = ESFConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  while (0 < _lock._writersActive || 0 < _lock._writersWaiting) {
    ++_lock._readersWaiting;

    error =
        ESFConvertError(pthread_cond_wait(&_lock._readSignal, &_lock._mutex));

    --_lock._readersWaiting;

    if (ESF_SUCCESS != error) {
      pthread_mutex_unlock(&_lock._mutex);

      return error;
    }
  }

  ++_lock._readersActive;

  return ESFConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no rw lock read lock function."
#endif
}

ESFError ESFReadWriteLock::writeAttempt() {
  if (ESF_MAGIC != _magic) {
    return ESF_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_RWLOCK_TRYWRLOCK

  int error = pthread_rwlock_trywrlock(&_lock);

  switch (error) {
    case 0:
      return ESF_SUCCESS;

    case EBUSY:
      return ESF_AGAIN;

    default:
      return ESFConvertError(error);
  }

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK

  ESFError error = ESFConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  if (0 < _lock._writersActive || 0 < _lock._readersActive) {
    error = ESFConvertError(pthread_mutex_unlock(&_lock._mutex));

    return ESF_SUCCESS == error ? ESF_AGAIN : error;
  }

  _lock._writersActive = 1;

  return ESFConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no rw lock try write lock function."
#endif
}

ESFError ESFReadWriteLock::readAttempt() {
  if (ESF_MAGIC != _magic) {
    return ESF_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_RWLOCK_TRYRDLOCK

  int error = pthread_rwlock_tryrdlock(&_lock);

  switch (error) {
    case 0:
      return ESF_SUCCESS;

    case EBUSY:
      return ESF_AGAIN;

    default:
      return ESFConvertError(error);
  }

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK

  ESFError error = ESFConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  if (0 < _lock._writersActive) {
    error = ESFConvertError(pthread_mutex_unlock(&_lock._mutex));

    return ESF_SUCCESS == error ? ESF_AGAIN : error;
  }

  ++_lock._readersActive;

  return ESFConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no rw lock try read lock function."
#endif
}

ESFError ESFReadWriteLock::writeRelease() {
  if (ESF_MAGIC != _magic) {
    return ESF_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_RWLOCK_UNLOCK

  return ESFConvertError(pthread_rwlock_unlock(&_lock));

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK && \
    defined HAVE_PTHREAD_COND_SIGNAL && defined HAVE_PTHREAD_COND_BROADCAST

  ESFError error = ESFConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  ESF_ASSERT(1 == _lock._writersActive);

  if (1 != _lock._writersActive) {
    pthread_mutex_unlock(&_lock._mutex);

    return ESF_INVALID_STATE;
  }

  _lock._writersActive = 0;

  error = ESF_SUCCESS;

  if (0 < _lock._writersWaiting) {
    error = ESFConvertError(pthread_cond_signal(&_lock._writeSignal));
  } else if (0 < _lock._readersWaiting) {
    error = ESFConvertError(pthread_cond_broadcast(&_lock._readSignal));
  }

  if (ESF_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  return ESFConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no rw lock write unlock function."
#endif
}

ESFError ESFReadWriteLock::readRelease() {
  if (ESF_MAGIC != _magic) {
    return ESF_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_RWLOCK_UNLOCK

  return ESFConvertError(pthread_rwlock_unlock(&_lock));

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK && \
    defined HAVE_PTHREAD_COND_SIGNAL && defined HAVE_PTHREAD_COND_BROADCAST

  ESFError error = ESFConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  ESF_ASSERT(0 == _lock._writersActive);
  ESF_ASSERT(0 < _lock._readersActive);

  if (0 < _lock._writersActive || 1 > _lock._readersActive) {
    pthread_mutex_unlock(&_lock._mutex);

    return ESF_INVALID_STATE;
  }

  --_lock._readersActive;

  if (0 == _lock._readersActive && 0 < _lock._writersWaiting) {
    error = ESFConvertError(pthread_cond_signal(&_lock._writeSignal));

    if (ESF_SUCCESS != error) {
      pthread_mutex_unlock(&_lock._mutex);
      return error;
    }
  }

  return ESFConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no rw lock read unlock function."
#endif
}
