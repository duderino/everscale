/** @file ESFThreadPool.cpp
 *  @brief A thread pool that executes ESFCommands
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

#ifndef ESF_THREAD_POOL_H
#include <ESFThreadPool.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_NULL_LOGGER_H
#include <ESFNullLogger.h>
#endif

#define MIN_THREADS 1

class ESFThreadPoolWorker : public ESFThread {
 public:
  ESFThreadPoolWorker(int workerId, const char *name,
                      ESFSharedEmbeddedQueue *queue, ESFLogger *logger);

  virtual ~ESFThreadPoolWorker();

  virtual void run();

 private:
  // Disabled
  ESFThreadPoolWorker(const ESFThreadPoolWorker &worker);
  void operator=(const ESFThreadPoolWorker &worker);

  int _workerId;
  const char *_name;
  ESFSharedEmbeddedQueue *_queue;
  ESFLogger *_logger;
};

ESFThreadPool::ESFThreadPool(const char *name, int threads, ESFLogger *logger,
                             ESFAllocator *allocator)
    : _numThreads(threads < MIN_THREADS ? MIN_THREADS : threads),
      _name(name ? name : "ThreadPool"),
      _threads(0),
      _allocator(allocator ? allocator : ESFSystemAllocator::GetInstance()),
      _logger(logger ? logger : ESFNullLogger::GetInstance()),
      _queue() {}

ESFThreadPool::~ESFThreadPool() {}

ESFError ESFThreadPool::start() {
  if (_logger->isLoggable(ESFLogger::Notice)) {
    _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[%s] starting", _name);
  }

  if (false == createWorkerThreads()) {
    if (_logger->isLoggable(ESFLogger::Critical)) {
      _logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                   "[%s] cannot start: Out Of Memory");
    }

    return ESF_OUT_OF_MEMORY;
  }

  ESFError error;

  for (int i = 0; i < _numThreads; ++i) {
    if (_logger->isLoggable(ESFLogger::Notice)) {
      _logger->log(ESFLogger::Notice, __FILE__, __LINE__,
                   "[%s] starting worker %d", _name, i + 1);
    }

    error = _threads[i]->start();

    if (ESF_SUCCESS != error) {
      if (_logger->isLoggable(ESFLogger::Critical)) {
        char buffer[100];

        ESFDescribeError(error, buffer, sizeof(buffer));

        _logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                     "[%s] cannot start worker %d: %s", _name, i + 1, buffer);
      }

      _numThreads = i;

      stop();

      return error;
    }
  }

  if (_logger->isLoggable(ESFLogger::Notice)) {
    _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[%s] started", _name);
  }

  return ESF_SUCCESS;
}

void ESFThreadPool::stop() {
  if (_logger->isLoggable(ESFLogger::Notice)) {
    _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[%s] stopping", _name);
  }

  // worker threads will get ESF_SHUTDOWN next time they try to pull an item
  // from the queue
  _queue.stop();

  // stop the worker threads if they are in the middle of running a command
  for (int i = 0; i < _numThreads; ++i) {
    _threads[i]->stop();
  }

  ESFError error;

  for (int i = 0; i < _numThreads; ++i) {
    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[%s] waiting for worker %d", _name, i + 1);
    }

    error = _threads[i]->join();

    if (ESF_SUCCESS != error) {
      if (_logger->isLoggable(ESFLogger::Warning)) {
        char buffer[100];

        ESFDescribeError(error, buffer, sizeof(buffer));

        _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                     "[%s] error joining worker %d: %s", _name, i + 1, buffer);
      }
    }
  }

  destroyWorkerThreads();

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__, "[%s] stopped", _name);
  }
}

bool ESFThreadPool::createWorkerThreads() {
  _threads =
      (ESFThread **)_allocator->allocate(_numThreads * sizeof(ESFThread *));

  if (!_threads) {
    return false;
  }

  for (int i = 0; i < _numThreads; ++i) {
    _threads[i] = 0;
  }

  for (int i = 0; i < _numThreads; ++i) {
    _threads[i] =
        new (_allocator) ESFThreadPoolWorker(i + 1, _name, &_queue, _logger);

    if (0 == _threads[i]) {
      destroyWorkerThreads();

      return false;
    }
  }

  return true;
}

void ESFThreadPool::destroyWorkerThreads() {
  if (!_threads) {
    return;
  }

  for (int i = 0; i < _numThreads; ++i) {
    if (0 == _threads[i]) {
      break;
    }

    _threads[i]->~ESFThread();
    _allocator->deallocate(_threads[i]);
  }

  _allocator->deallocate(_threads);
  _threads = 0;
}

ESFThreadPoolWorker::ESFThreadPoolWorker(int workerId, const char *name,
                                         ESFSharedEmbeddedQueue *queue,
                                         ESFLogger *logger)
    : _workerId(workerId), _name(name), _queue(queue), _logger(logger) {}

ESFThreadPoolWorker::~ESFThreadPoolWorker() {}

void ESFThreadPoolWorker::run() {
  if (_logger->isLoggable(ESFLogger::Notice)) {
    _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[%s:%d] starting",
                 _name, _workerId);
  }

  ESFError error;
  ESFCommand *command = 0;
  ESFCleanupHandler *cleanupHandler = 0;
  int errorCount = 0;
  bool cleanup = false;

  while (isRunning() && errorCount < 10) {
    command = (ESFCommand *)_queue->pop(&error);

    if (ESF_SHUTDOWN == error) {
      if (_logger->isLoggable(ESFLogger::Debug)) {
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[%s:%d] received shutdown command", _name, _workerId);
      }

      break;
    }

    if (ESF_SUCCESS != error) {
      if (_logger->isLoggable(ESFLogger::Error)) {
        char buffer[100];

        ESFDescribeError(error, buffer, sizeof(buffer));

        _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                     "[%s:%d] cannot pop command from queue: %s", _name,
                     _workerId, buffer);
      }

      ++errorCount;

      continue;
    }

    // todo add performance counters

    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[%s:%d] starting command %s", _name, _workerId,
                   command->getName());
    }

    cleanup = command->run(&_isRunning);

    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[%s:%d] finished command %s", _name, _workerId,
                   command->getName());
    }

    if (cleanup) {
      cleanupHandler = command->getCleanupHandler();

      if (cleanupHandler) {
        cleanupHandler->destroy(command);
      }
    }

    command = 0;
    errorCount = 0;
  }

  if (_logger->isLoggable(ESFLogger::Notice)) {
    _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[%s:%d] exiting",
                 _name, _workerId);
  }
}
