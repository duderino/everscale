#ifndef ESB_READ_WRITE_LOCK_H
#include <ESBReadWriteLock.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

namespace ESB {

ReadWriteLock::ReadWriteLock() : _magic(0) {
#ifdef HAVE_PTHREAD_RWLOCK_INIT

  if (0 == pthread_rwlock_init(&_lock, 0)) {
    _magic = ESB_MAGIC;
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

  _magic = ESB_MAGIC;

#else
#error "Platform has no rw lock initializer"
#endif
}

ReadWriteLock::~ReadWriteLock() {
  if (ESB_MAGIC != _magic) {
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

Error ReadWriteLock::writeAcquire() {
  if (ESB_MAGIC != _magic) {
    return ESB_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_RWLOCK_WRLOCK

  return ConvertError(pthread_rwlock_wrlock(&_lock));

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_COND_WAIT && \
    defined HAVE_PTHREAD_MUTEX_UNLOCK

  Error error = ConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  while (0 < _lock._writersActive || 0 < _lock._readersActive) {
    ++_lock._writersWaiting;

    error = ConvertError(pthread_cond_wait(&_lock._writeSignal, &_lock._mutex));

    --_lock._writersWaiting;

    if (ESB_SUCCESS != error) {
      pthread_mutex_unlock(&_lock._mutex);

      return error;
    }
  }

  assert(0 == _lock._writersActive && 0 == _lock._readersActive);

  _lock._writersActive = 1;

  return ConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no rw lock write lock function."
#endif
}

Error ReadWriteLock::readAcquire() {
  if (ESB_MAGIC != _magic) {
    return ESB_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_RWLOCK_RDLOCK

  return ConvertError(pthread_rwlock_rdlock(&_lock));

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_COND_WAIT && \
    defined HAVE_PTHREAD_MUTEX_UNLOCK

  Error error = ConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  while (0 < _lock._writersActive || 0 < _lock._writersWaiting) {
    ++_lock._readersWaiting;

    error = ConvertError(pthread_cond_wait(&_lock._readSignal, &_lock._mutex));

    --_lock._readersWaiting;

    if (ESB_SUCCESS != error) {
      pthread_mutex_unlock(&_lock._mutex);

      return error;
    }
  }

  ++_lock._readersActive;

  return ConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no rw lock read lock function."
#endif
}

Error ReadWriteLock::writeAttempt() {
  if (ESB_MAGIC != _magic) {
    return ESB_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_RWLOCK_TRYWRLOCK

  int error = pthread_rwlock_trywrlock(&_lock);

  switch (error) {
    case 0:
      return ESB_SUCCESS;

    case EBUSY:
      return ESB_AGAIN;

    default:
      return ConvertError(error);
  }

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK

  Error error = ConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  if (0 < _lock._writersActive || 0 < _lock._readersActive) {
    error = ConvertError(pthread_mutex_unlock(&_lock._mutex));

    return ESB_SUCCESS == error ? ESB_AGAIN : error;
  }

  _lock._writersActive = 1;

  return ConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no rw lock try write lock function."
#endif
}

Error ReadWriteLock::readAttempt() {
  if (ESB_MAGIC != _magic) {
    return ESB_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_RWLOCK_TRYRDLOCK

  int error = pthread_rwlock_tryrdlock(&_lock);

  switch (error) {
    case 0:
      return ESB_SUCCESS;

    case EBUSY:
      return ESB_AGAIN;

    default:
      return ConvertError(error);
  }

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK

  Error error = ConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  if (0 < _lock._writersActive) {
    error = ConvertError(pthread_mutex_unlock(&_lock._mutex));

    return ESB_SUCCESS == error ? ESB_AGAIN : error;
  }

  ++_lock._readersActive;

  return ConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no rw lock try read lock function."
#endif
}

Error ReadWriteLock::writeRelease() {
  if (ESB_MAGIC != _magic) {
    return ESB_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_RWLOCK_UNLOCK

  return ConvertError(pthread_rwlock_unlock(&_lock));

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK && \
    defined HAVE_PTHREAD_COND_SIGNAL && defined HAVE_PTHREAD_COND_BROADCAST

  Error error = ConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  assert(1 == _lock._writersActive);

  if (1 != _lock._writersActive) {
    pthread_mutex_unlock(&_lock._mutex);

    return ESB_INVALID_STATE;
  }

  _lock._writersActive = 0;

  error = ESB_SUCCESS;

  if (0 < _lock._writersWaiting) {
    error = ConvertError(pthread_cond_signal(&_lock._writeSignal));
  } else if (0 < _lock._readersWaiting) {
    error = ConvertError(pthread_cond_broadcast(&_lock._readSignal));
  }

  if (ESB_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  return ConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no rw lock write unlock function."
#endif
}

Error ReadWriteLock::readRelease() {
  if (ESB_MAGIC != _magic) {
    return ESB_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_RWLOCK_UNLOCK

  return ConvertError(pthread_rwlock_unlock(&_lock));

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK && \
    defined HAVE_PTHREAD_COND_SIGNAL && defined HAVE_PTHREAD_COND_BROADCAST

  Error error = ConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  assert(0 == _lock._writersActive);
  assert(0 < _lock._readersActive);

  if (0 < _lock._writersActive || 1 > _lock._readersActive) {
    pthread_mutex_unlock(&_lock._mutex);

    return ESB_INVALID_STATE;
  }

  --_lock._readersActive;

  if (0 == _lock._readersActive && 0 < _lock._writersWaiting) {
    error = ConvertError(pthread_cond_signal(&_lock._writeSignal));

    if (ESB_SUCCESS != error) {
      pthread_mutex_unlock(&_lock._mutex);
      return error;
    }
  }

  return ConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no rw lock read unlock function."
#endif
}

}  // namespace ESB
