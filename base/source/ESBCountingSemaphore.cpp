#ifndef ESB_COUNTING_SEMAPHORE_H
#include <ESBCountingSemaphore.h>
#endif

namespace ESB {

CountingSemaphore::CountingSemaphore() : _magic(0) {
#if defined HAVE_SEM_INIT && 2147483647 == SEM_VALUE_MAX

  if (0 == sem_init(&_semaphore, 0, 0)) {
    _magic = ESB_MAGIC;
  }

#elif defined HAVE_PTHREAD_MUTEX_INIT && defined HAVE_PTHREAD_COND_INIT && defined HAVE_PTHREAD_MUTEX_DESTROY

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

  _magic = ESB_MAGIC;
}

CountingSemaphore::~CountingSemaphore() {
  if (ESB_MAGIC != _magic) {
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

Error CountingSemaphore::writeAcquire() {
  if (ESB_MAGIC != _magic) {
    return ESB_NOT_INITIALIZED;
  }

#if defined HAVE_SEM_WAIT && 2147483647 == SEM_VALUE_MAX

  Error error;

  while (true) {
    if (0 == sem_wait(&_semaphore)) {
      return ESB_SUCCESS;
    }

    error = LastError();

    if (ESB_INTR == error) {
      continue;
    }

    return error;
  }

  return ESB_SUCCESS;

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_COND_WAIT && defined HAVE_PTHREAD_MUTEX_UNLOCK

  Error error;

  error = ConvertError(pthread_mutex_lock(&_semaphore._mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  while (1 > _semaphore._count) {
    error = ConvertError(pthread_cond_wait(&_semaphore._cond, &_semaphore._mutex));

    if (ESB_SUCCESS != error && ESB_INTR != error) {
      return error;
    }
  }

  --_semaphore._count;

  return ConvertError(pthread_mutex_unlock(&_semaphore._mutex));

#elif defined HAVE_WAIT_FOR_SINGLE_OBJECT

  switch (WaitForSingleObject(_semaphore, INFINITE)) {
    case WAIT_OBJECT_0:
      return ESB_SUCCESS;
    case WAIT_TIMEOUT:
      return ESB_AGAIN;
    case WAIT_FAILED:
      return LastError();
    default:
      return ESB_OTHER_ERROR;
  }

#else
#error "Platform has no counting semaphore write lock function."
#endif
}

Error CountingSemaphore::readAcquire() { return writeAcquire(); }

Error CountingSemaphore::writeAttempt() {
  if (ESB_MAGIC != _magic) {
    return ESB_NOT_INITIALIZED;
  }

#if defined HAVE_SEM_TRYWAIT && 2147483647 == SEM_VALUE_MAX

  if (0 == sem_trywait(&_semaphore)) {
    return ESB_SUCCESS;
  }

  // GetLastError will map EAGAIN to ESB_AGAIN
  return LastError();

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK

  Error error;

  error = ConvertError(pthread_mutex_lock(&_semaphore._mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  if (0 < _semaphore._count) {
    --_semaphore._count;

    return ConvertError(pthread_mutex_unlock(&_semaphore._mutex));
  }

  error = pthread_mutex_unlock(&_semaphore._mutex);

  return ESB_SUCCESS == error ? ESB_AGAIN : error;

#elif defined HAVE_WAIT_FOR_SINGLE_OBJECT

  switch (WaitForSingleObject(_semaphore, 0)) {
    case WAIT_OBJECT_0:
      return ESB_SUCCESS;
    case WAIT_TIMEOUT:
      return ESB_AGAIN;
    case WAIT_FAILED:
      return LastError();
    default:
      return ESB_OTHER_ERROR;
  }

#else
#error "Platform has no counting semaphore write trylock function."
#endif
}

Error CountingSemaphore::readAttempt() { return writeAttempt(); }

Error CountingSemaphore::writeRelease() {
  if (ESB_MAGIC != _magic) {
    return ESB_NOT_INITIALIZED;
  }

#if defined HAVE_SEM_POST && 2147483647 == SEM_VALUE_MAX

  if (0 == sem_post(&_semaphore)) {
    return ESB_SUCCESS;
  }

  return LastError();

#elif defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK && defined HAVE_PTHREAD_COND_SIGNAL

  Error error;

  error = ConvertError(pthread_mutex_lock(&_semaphore._mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  if (ESB_INT32_MAX == _semaphore._count) {
    pthread_mutex_unlock(&_semaphore._mutex);

    return ESB_OVERFLOW;
  }

  ++_semaphore._count;

  error = ConvertError(pthread_cond_signal(&_semaphore._cond));

  if (ESB_SUCCESS != error) {
    pthread_mutex_unlock(&_semaphore._mutex);

    return error;
  }

  return ConvertError(pthread_mutex_unlock(&_semaphore._mutex));

#elif defined HAVE_RELEASE_SEMAPHORE

  if (0 != ReleaseSemaphore(_semaphore, 1, 0)) {
    return ESB_SUCCESS;
  }

  return LastError();

#else
#error "Platform has no counting semaphore write unlock function."
#endif
}

Error CountingSemaphore::readRelease() { return writeRelease(); }

}  // namespace ESB
