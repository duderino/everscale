#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

namespace ESB {

Mutex::Mutex() : _magic(0) {
#ifdef HAVE_PTHREAD_MUTEX_INIT

  int error = pthread_mutex_init(&_mutex, 0);

  assert(0 == error);

  if (0 == error) {
    _magic = ESB_MAGIC;
  }

#elif defined HAVE_CREATEMUTEX

  _mutex = CreateMutex(0, false, 0);

  if (_mutex) {
    _magic = ESB_MAGIC;
  }

#else
#error "Platform has no mutex initializer"
#endif
}

Mutex::~Mutex() {
  if (ESB_MAGIC != _magic) {
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

Error Mutex::writeAcquire() {
  if (ESB_MAGIC != _magic) {
    return ESB_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_MUTEX_LOCK

  return ConvertError(pthread_mutex_lock(&_mutex));

#elif defined HAVE_WAIT_FOR_SINGLE_OBJECT

  switch (WaitForSingleObject(_mutex, INFINITE)) {
    case WAIT_OBJECT_0:

      return ESB_SUCCESS;

    case WAIT_TIMEOUT:

      return ESB_AGAIN;

    case WAIT_FAILED:

      return GetLastError();

    default:

      return ESB_OTHER_ERROR;
  }

#else
#error "Platform has no mutex lock function."
#endif
}

Error Mutex::readAcquire() { return writeAcquire(); }

Error Mutex::writeAttempt() {
  if (ESB_MAGIC != _magic) {
    return ESB_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_MUTEX_TRYLOCK

  int error = pthread_mutex_trylock(&_mutex);

  switch (error) {
    case 0:
      return ESB_SUCCESS;

    case EBUSY:
      return ESB_AGAIN;

    default:
      return ConvertError(error);
  }

#elif defined HAVE_WAIT_FOR_SINGLE_OBJECT

  switch (WaitForSingleObject(_mutex, 0)) {
    case WAIT_OBJECT_0:

      return ESB_SUCCESS;

    case WAIT_TIMEOUT:

      return ESB_AGAIN;

    case WAIT_FAILED:

      return GetLastError();

    default:

      return ESB_OTHER_ERROR;
  }

#else
#error "Platform has no mutex trylock function."
#endif
}

Error Mutex::readAttempt() { return writeAttempt(); }

Error Mutex::writeRelease() {
  if (ESB_MAGIC != _magic) {
    return ESB_NOT_INITIALIZED;
  }

#ifdef HAVE_PTHREAD_MUTEX_UNLOCK

  return ConvertError(pthread_mutex_unlock(&_mutex));

#elif defined HAVE_RELEASEMUTEX

  if (0 == ReleaseMutex(_mutex)) {
    return GetLastError();
  }

  return ESB_SUCCESS;

#else
#error "Platform has no mutex unlock function."
#endif
}

Error Mutex::readRelease() { return writeRelease(); }

}  // namespace ESB
