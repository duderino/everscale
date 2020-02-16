/** @file ESFSharedQueue.cpp
 *  @brief A threadsafe queue good for producer/consumer problems
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

#ifndef ESF_SHARED_QUEUE_H
#include <ESFSharedQueue.h>
#endif

#ifndef ESF_NULL_LOCK_H
#include <ESFNullLock.h>
#endif

ESFSharedQueue::Synchronizer::Synchronizer() : _magic(0) {
#if defined HAVE_PTHREAD_MUTEX_INIT && defined HAVE_PTHREAD_COND_INIT && \
    defined HAVE_PTHREAD_MUTEX_DESTROY && defined HAVE_PTHREAD_COND_DESTROY

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

  _magic = ESF_MAGIC;
}

ESFSharedQueue::Synchronizer::~Synchronizer() {
  if (ESF_MAGIC != _magic) {
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

ESFSharedQueue::ESFSharedQueue(ESFAllocator *allocator, ESFUInt32 limit)
    : _limit(limit), _lock(), _list(allocator, ESFNullLock::Instance()) {}

ESFSharedQueue::~ESFSharedQueue() {}

ESFError ESFSharedQueue::push(void *item) {
  if (!item) {
    return ESF_NULL_POINTER;
  }

  if (ESF_MAGIC != _lock._magic) {
    return ESF_NOT_INITIALIZED;
  }

#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_COND_WAIT && \
    defined HAVE_PTHREAD_MUTEX_UNLOCK && defined HAVE_PTHREAD_COND_SIGNAL

  ESFError error = ESFConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  while (0 != _limit && _limit <= _list.getSize()) {
    error = ESFConvertError(
        pthread_cond_wait(&_lock._producerSignal, &_lock._mutex));

    if (ESF_SUCCESS != error && ESF_INTR != error) {
      return error;
    }
  }

  error = _list.pushBack(item);

  if (ESF_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  error = ESFConvertError(pthread_cond_signal(&_lock._consumerSignal));

  if (ESF_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  return ESFConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no shared queue push function"
#endif
}

ESFError ESFSharedQueue::tryPush(void *item) {
  if (!item) {
    return ESF_NULL_POINTER;
  }

  if (ESF_MAGIC != _lock._magic) {
    return ESF_NOT_INITIALIZED;
  }

#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK && \
    defined HAVE_PTHREAD_COND_SIGNAL

  ESFError error = ESFConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  if (0 != _limit && _limit <= _list.getSize()) {
    error = pthread_mutex_unlock(&_lock._mutex);

    return ESF_SUCCESS == error ? ESF_AGAIN : error;
  }

  error = _list.pushBack(item);

  if (ESF_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  error = ESFConvertError(pthread_cond_signal(&_lock._consumerSignal));

  if (ESF_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  return ESFConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no shared queue try push function"
#endif
}

ESFError ESFSharedQueue::pop(void **item) {
  if (!item) {
    return ESF_NULL_POINTER;
  }

  if (ESF_MAGIC != _lock._magic) {
    return ESF_NOT_INITIALIZED;
  }

#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_COND_SIGNAL && \
    defined HAVE_PTHREAD_MUTEX_UNLOCK && defined HAVE_PTHREAD_COND_WAIT

  ESFError error = ESFConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  while (_list.isEmpty()) {
    error = ESFConvertError(
        pthread_cond_wait(&_lock._consumerSignal, &_lock._mutex));

    if (ESF_SUCCESS != error && ESF_INTR != error) {
      return error;
    }
  }

  ESFUInt32 size = _list.getSize();

  void *tmp = _list.getFront();

  error = _list.popFront();

  if (ESF_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  *item = tmp;

  if (_limit == size) {
    error = ESFConvertError(pthread_cond_broadcast(&_lock._producerSignal));

    if (ESF_SUCCESS != error) {
      pthread_mutex_unlock(&_lock._mutex);

      return error;
    }
  }

  return ESFConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no shared queue pop function"
#endif
}

ESFError ESFSharedQueue::tryPop(void **item) {
  if (!item) {
    return ESF_NULL_POINTER;
  }

  if (ESF_MAGIC != _lock._magic) {
    return ESF_NOT_INITIALIZED;
  }

#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_COND_SIGNAL && \
    defined HAVE_PTHREAD_MUTEX_UNLOCK

  ESFError error = ESFConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  ESFUInt32 size = _list.getSize();

  if (0 == size) {
    error = ESFConvertError(pthread_mutex_unlock(&_lock._mutex));

    return ESF_SUCCESS == error ? ESF_AGAIN : error;
  }

  void *tmp = _list.getFront();

  error = _list.popFront();

  if (ESF_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  *item = tmp;

  if (_limit == size) {
    error = ESFConvertError(pthread_cond_broadcast(&_lock._producerSignal));

    if (ESF_SUCCESS != error) {
      pthread_mutex_unlock(&_lock._mutex);

      return error;
    }
  }

  return ESFConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no shared queue try pop function"
#endif
}

ESFError ESFSharedQueue::clear() {
  if (ESF_MAGIC != _lock._magic) {
    return ESF_NOT_INITIALIZED;
  }

#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_COND_BROADCAST && \
    defined HAVE_PTHREAD_MUTEX_UNLOCK

  ESFError error = ESFConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  ESFUInt32 size = _list.getSize();

  error = _list.clear();

  if (ESF_SUCCESS != error) {
    pthread_mutex_unlock(&_lock._mutex);

    return error;
  }

  if (_limit == (size + ESF_UINT32_C(1))) {
    error = ESFConvertError(pthread_cond_broadcast(&_lock._producerSignal));

    if (ESF_SUCCESS != error) {
      pthread_mutex_unlock(&_lock._mutex);

      return error;
    }
  }

  return ESFConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no shared queue clear method"
#endif
}

ESFError ESFSharedQueue::getSize(ESFUInt32 *size) {
  if (!size) {
    return ESF_NULL_POINTER;
  }

  if (ESF_MAGIC != _lock._magic) {
    return ESF_NOT_INITIALIZED;
  }

#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK

  ESFError error = ESFConvertError(pthread_mutex_lock(&_lock._mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  *size = _list.getSize();

  return ESFConvertError(pthread_mutex_unlock(&_lock._mutex));

#else
#error "Platform has no shared queue getSize method"
#endif
}

ESFSize ESFSharedQueue::GetAllocationSize() {
  return ESFList::GetAllocationSize();
}
