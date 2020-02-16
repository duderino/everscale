/** @file ESFSharedEmbeddedQueue.cpp
 *  @brief A producer/consumer queue of ESFEmbeddedListElements
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_SHARED_EMBEDDED_QUEUE_H
#include <ESFSharedEmbeddedQueue.h>
#endif

ESFSharedEmbeddedQueue::ESFSharedEmbeddedQueue() : _isStopped(false), _list() {
#if defined HAVE_PTHREAD_MUTEX_INIT && defined HAVE_PTHREAD_COND_INIT && \
    defined HAVE_PTHREAD_MUTEX_DESTROY && defined HAVE_PTHREAD_COND_DESTROY

  pthread_mutex_init(&_mutex, 0);
  pthread_cond_init(&_signal, 0);

#else
#error "Platform has no mutex or signal initializer"
#endif
}

ESFSharedEmbeddedQueue::~ESFSharedEmbeddedQueue() {
#if defined HAVE_PTHREAD_MUTEX_DESTROY && defined HAVE_PTHREAD_COND_DESTROY

  pthread_mutex_destroy(&_mutex);
  pthread_cond_destroy(&_signal);

#else
#error "Platform has no mutex or signal destructor"
#endif
}

ESFError ESFSharedEmbeddedQueue::push(ESFEmbeddedListElement *element) {
#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK && \
    defined HAVE_PTHREAD_COND_SIGNAL

  ESFError error = ESFConvertError(pthread_mutex_lock(&_mutex));

  if (ESF_SUCCESS != error) {
    return error;
  }

  if (_isStopped) {
    pthread_mutex_unlock(&_mutex);

    return ESF_SHUTDOWN;
  }

  _list.addLast(element);

  pthread_mutex_unlock(&_mutex);
  pthread_cond_signal(&_signal);

#else
#error "Platform has no mutex lock, mutex unlock, or cond signal"
#endif

  return ESF_SUCCESS;
}

ESFEmbeddedListElement *ESFSharedEmbeddedQueue::pop(ESFError *result) {
  ESFEmbeddedListElement *tmp = 0;

  if (result) *result = ESF_SUCCESS;

#if defined HAVE_PTHREAD_MUTEX_LOCK && defined HAVE_PTHREAD_MUTEX_UNLOCK && \
    defined HAVE_PTHREAD_COND_WAIT

  ESFError error = ESFConvertError(pthread_mutex_lock(&_mutex));

  if (ESF_SUCCESS != error) {
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

    error = ESFConvertError(pthread_cond_wait(&_signal, &_mutex));

    if (ESF_SUCCESS != error && ESF_INTR != error) {
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
    *result = ESF_SHUTDOWN;
  }

  return 0;
}

void ESFSharedEmbeddedQueue::stop() {
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
