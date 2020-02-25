#ifndef ESB_SHARED_EMBEDDED_QUEUE_H
#include <ESBSharedEmbeddedQueue.h>
#endif

namespace ESB {

SharedEmbeddedQueue::SharedEmbeddedQueue() : _isStopped(false), _list() {
#if defined HAVE_PTHREAD_MUTEX_INIT && defined HAVE_PTHREAD_COND_INIT && \
    defined HAVE_PTHREAD_MUTEX_DESTROY && defined HAVE_PTHREAD_COND_DESTROY

  pthread_mutex_init(&_mutex, 0);
  pthread_cond_init(&_signal, 0);

#else
#error "Platform has no mutex or signal initializer"
#endif
}

SharedEmbeddedQueue::~SharedEmbeddedQueue() {
#if defined HAVE_PTHREAD_MUTEX_DESTROY && defined HAVE_PTHREAD_COND_DESTROY

  pthread_mutex_destroy(&_mutex);
  pthread_cond_destroy(&_signal);

#else
#error "Platform has no mutex or signal destructor"
#endif
}

Error SharedEmbeddedQueue::push(EmbeddedListElement *element) {
#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK && \
    defined HAVE_PTHREAD_COND_SIGNAL

  Error error = ConvertError(pthread_mutex_lock(&_mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  if (_isStopped) {
    pthread_mutex_unlock(&_mutex);

    return ESB_SHUTDOWN;
  }

  _list.addLast(element);

  pthread_mutex_unlock(&_mutex);
  pthread_cond_signal(&_signal);

#else
#error "Platform has no mutex lock, mutex unlock, or cond signal"
#endif

  return ESB_SUCCESS;
}

EmbeddedListElement *SharedEmbeddedQueue::pop(Error *result) {
  EmbeddedListElement *tmp = 0;

  if (result) *result = ESB_SUCCESS;

#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK && \
    defined HAVE_PTHREAD_COND_WAIT

  Error error = ConvertError(pthread_mutex_lock(&_mutex));

  if (ESB_SUCCESS != error) {
    if (result) *result = error;
    return 0;
  }

  do {
    if (_isStopped) {
      break;
    }

    tmp = _list.removeFirst();

    if (tmp) {
      break;
    }

    error = ConvertError(pthread_cond_wait(&_signal, &_mutex));

    if (ESB_SUCCESS != error && ESB_INTR != error) {
      if (result) *result = error;
      return 0;
    }
  } while (0 == tmp);

  pthread_mutex_unlock(&_mutex);

#else
#error "Platform has no mutex lock, mutex unlock, or cond wait"
#endif

  if (tmp) {
    return tmp;
  }

  if (result) {
    *result = ESB_SHUTDOWN;
  }

  return 0;
}

void SharedEmbeddedQueue::stop() {
#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK && \
    defined HAVE_PTHREAD_COND_BROADCAST

  pthread_mutex_lock(&_mutex);

  _isStopped = true;
  _list.clear(true);

  pthread_mutex_unlock(&_mutex);
  pthread_cond_broadcast(&_signal);

#else
#error "Platform has no mutex lock, mutex unlock, or cond broadcast"
#endif
}

}  // namespace ESB
