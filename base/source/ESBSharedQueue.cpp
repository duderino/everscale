#ifndef ESB_SHARED_QUEUE_H
#include <ESBSharedQueue.h>
#endif

#ifndef ESB_NULL_LOCK_H
#include <ESBNullLock.h>
#endif

namespace ESB {

SharedQueue::Synchronizer::Synchronizer() : _magic(0) {
#if defined HAVE_PTHREAD_MUTEX_INIT && defined HAVE_PTHREAD_COND_INIT && defined HAVE_PTHREAD_MUTEX_DESTROY && \
    defined HAVE_PTHREAD_COND_DESTROY

  if (0 != pthread_mutex_init(&_mutex, 0)) {
    return;
  }

  if (0 != pthread_cond_init(&_consumerSignal, 0)) {
    pthread_mutex_destroy(&_mutex);
    return;
  }

  if (0 != pthread_cond_init(&_producerSignal, 0)) {
    pthread_cond_destroy(&_consumerSignal);
    pthread_mutex_destroy(&_mutex);
    return;
  }

#else
#error "Platform has no mutex/signal initializer"
#endif

  _magic = ESB_MAGIC;
}

SharedQueue::Synchronizer::~Synchronizer() {
  if (ESB_MAGIC != _magic) {
    return;
  }

#if defined HAVE_PTHREAD_MUTEX_DESTROY && defined HAVE_PTHREAD_COND_DESTROY

  pthread_mutex_destroy(&_mutex);
  pthread_cond_destroy(&_consumerSignal);
  pthread_cond_destroy(&_producerSignal);

#else
#error "Platform has no mutex/signal destructor"
#endif
}

SharedQueue::SharedQueue(UInt32 limit, Allocator &allocator)
    : _limit(limit), _lock(), _list(NullLock::Instance(), allocator) {}

SharedQueue::~SharedQueue() {}

Error SharedQueue::push(void *item) {
  if (!item) {
    return ESB_NULL_POINTER;
  }

  if (ESB_MAGIC != _lock._magic) {
    return ESB_NOT_INITIALIZED;
  }

#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_COND_WAIT && defined HAVE_PTHREAD_MUTEX_UNLOCK && \
    defined HAVE_PTHREAD_COND_SIGNAL

  Error error = ConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  while (0 != _limit && _limit <= _list.size()) {
    error = ConvertError(pthread_cond_wait(&_lock._producerSignal, &_lock._mutex));

    if (ESB_SUCCESS != error && ESB_INTR != error) {
      return error;
    }
  }

  error = _list.pushBack(item);

  if (ESB_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  error = ConvertError(pthread_cond_signal(&_lock._consumerSignal));

  if (ESB_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  return ConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no shared queue push function"
#endif
}

Error SharedQueue::tryPush(void *item) {
  if (!item) {
    return ESB_NULL_POINTER;
  }

  if (ESB_MAGIC != _lock._magic) {
    return ESB_NOT_INITIALIZED;
  }

#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK && defined HAVE_PTHREAD_COND_SIGNAL

  Error error = ConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  if (0 != _limit && _limit <= _list.size()) {
    error = pthread_mutex_unlock(&_lock._mutex);

    return ESB_SUCCESS == error ? ESB_AGAIN : error;
  }

  error = _list.pushBack(item);

  if (ESB_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  error = ConvertError(pthread_cond_signal(&_lock._consumerSignal));

  if (ESB_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  return ConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no shared queue try push function"
#endif
}

Error SharedQueue::pop(void **item) {
  if (!item) {
    return ESB_NULL_POINTER;
  }

  if (ESB_MAGIC != _lock._magic) {
    return ESB_NOT_INITIALIZED;
  }

#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_COND_SIGNAL && defined HAVE_PTHREAD_MUTEX_UNLOCK && \
    defined HAVE_PTHREAD_COND_WAIT

  Error error = ConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  while (_list.isEmpty()) {
    error = ConvertError(pthread_cond_wait(&_lock._consumerSignal, &_lock._mutex));

    if (ESB_SUCCESS != error && ESB_INTR != error) {
      return error;
    }
  }

  UInt32 size = _list.size();

  void *tmp = _list.front();

  error = _list.popFront();

  if (ESB_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  *item = tmp;

  if (_limit == size) {
    error = ConvertError(pthread_cond_broadcast(&_lock._producerSignal));

    if (ESB_SUCCESS != error) {
      pthread_mutex_unlock(&_lock._mutex);

      return error;
    }
  }

  return ConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no shared queue pop function"
#endif
}

Error SharedQueue::tryPop(void **item) {
  if (!item) {
    return ESB_NULL_POINTER;
  }

  if (ESB_MAGIC != _lock._magic) {
    return ESB_NOT_INITIALIZED;
  }

#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_COND_SIGNAL && defined HAVE_PTHREAD_MUTEX_UNLOCK

  Error error = ConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  UInt32 size = _list.size();

  if (0 == size) {
    error = ConvertError(pthread_mutex_unlock(&_lock._mutex));

    return ESB_SUCCESS == error ? ESB_AGAIN : error;
  }

  void *tmp = _list.front();

  error = _list.popFront();

  if (ESB_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  *item = tmp;

  if (_limit == size) {
    error = ConvertError(pthread_cond_broadcast(&_lock._producerSignal));

    if (ESB_SUCCESS != error) {
      pthread_mutex_unlock(&_lock._mutex);

      return error;
    }
  }

  return ConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no shared queue try pop function"
#endif
}

Error SharedQueue::clear() {
  if (ESB_MAGIC != _lock._magic) {
    return ESB_NOT_INITIALIZED;
  }

#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_COND_BROADCAST && defined HAVE_PTHREAD_MUTEX_UNLOCK

  Error error = ConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  UInt32 size = _list.size();

  error = _list.clear();

  if (ESB_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  if (_limit == (size + ESB_UINT32_C(1))) {
    error = ConvertError(pthread_cond_broadcast(&_lock._producerSignal));

    if (ESB_SUCCESS != error) {
      pthread_mutex_unlock(&_lock._mutex);

      return error;
    }
  }

  return ConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no shared queue clear method"
#endif
}

Error SharedQueue::size(UInt32 *size) {
  if (!size) {
    return ESB_NULL_POINTER;
  }

  if (ESB_MAGIC != _lock._magic) {
    return ESB_NOT_INITIALIZED;
  }

#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK

  Error error = ConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESB_SUCCESS != error) {
    return error;
  }

  *size = _list.size();

  return ConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no shared queue getSize method"
#endif
}

Size SharedQueue::AllocationSize() { return List::AllocationSize(); }

}  // namespace ESB
