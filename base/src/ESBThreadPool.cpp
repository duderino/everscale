#ifndef ESB_THREAD_POOL_H
#include <ESBThreadPool.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

namespace ESB {

#define MIN_THREADS 1

class ThreadPoolWorker : public Thread {
 public:
  ThreadPoolWorker(int workerId, const char *name, SharedEmbeddedQueue *queue,
                   Logger *logger);

  virtual ~ThreadPoolWorker();

  virtual void run();

 private:
  // Disabled
  ThreadPoolWorker(const ThreadPoolWorker &worker);
  void operator=(const ThreadPoolWorker &worker);

  int _workerId;
  const char *_name;
  SharedEmbeddedQueue *_queue;
  Logger *_logger;
};

ThreadPool::ThreadPool(const char *name, int threads, Logger *logger,
                       Allocator *allocator)
    : _numThreads(threads < MIN_THREADS ? MIN_THREADS : threads),
      _name(name ? name : "ThreadPool"),
      _threads(0),
      _allocator(allocator ? allocator : SystemAllocator::GetInstance()),
      _logger(logger ? logger : NullLogger::GetInstance()),
      _queue() {}

ThreadPool::~ThreadPool() {}

Error ThreadPool::start() {
  if (_logger->isLoggable(Logger::Notice)) {
    _logger->log(Logger::Notice, __FILE__, __LINE__, "[%s] starting", _name);
  }

  if (false == createWorkerThreads()) {
    if (_logger->isLoggable(Logger::Critical)) {
      _logger->log(Logger::Critical, __FILE__, __LINE__,
                   "[%s] cannot start: Out Of Memory");
    }

    return ESB_OUT_OF_MEMORY;
  }

  Error error;

  for (int i = 0; i < _numThreads; ++i) {
    if (_logger->isLoggable(Logger::Notice)) {
      _logger->log(Logger::Notice, __FILE__, __LINE__,
                   "[%s] starting worker %d", _name, i + 1);
    }

    error = _threads[i]->start();

    if (ESB_SUCCESS != error) {
      if (_logger->isLoggable(Logger::Critical)) {
        char buffer[100];

        DescribeError(error, buffer, sizeof(buffer));

        _logger->log(Logger::Critical, __FILE__, __LINE__,
                     "[%s] cannot start worker %d: %s", _name, i + 1, buffer);
      }

      _numThreads = i;

      stop();

      return error;
    }
  }

  if (_logger->isLoggable(Logger::Notice)) {
    _logger->log(Logger::Notice, __FILE__, __LINE__, "[%s] started", _name);
  }

  return ESB_SUCCESS;
}

void ThreadPool::stop() {
  if (_logger->isLoggable(Logger::Notice)) {
    _logger->log(Logger::Notice, __FILE__, __LINE__, "[%s] stopping", _name);
  }

  // worker threads will get ESB_SHUTDOWN next time they try to pull an item
  // from the queue
  _queue.stop();

  // stop the worker threads if they are in the middle of running a command
  for (int i = 0; i < _numThreads; ++i) {
    _threads[i]->stop();
  }

  Error error;

  for (int i = 0; i < _numThreads; ++i) {
    if (_logger->isLoggable(Logger::Debug)) {
      _logger->log(Logger::Debug, __FILE__, __LINE__,
                   "[%s] waiting for worker %d", _name, i + 1);
    }

    error = _threads[i]->join();

    if (ESB_SUCCESS != error) {
      if (_logger->isLoggable(Logger::Warning)) {
        char buffer[100];

        DescribeError(error, buffer, sizeof(buffer));

        _logger->log(Logger::Warning, __FILE__, __LINE__,
                     "[%s] error joining worker %d: %s", _name, i + 1, buffer);
      }
    }
  }

  destroyWorkerThreads();

  if (_logger->isLoggable(Logger::Debug)) {
    _logger->log(Logger::Debug, __FILE__, __LINE__, "[%s] stopped", _name);
  }
}

bool ThreadPool::createWorkerThreads() {
  _threads = (Thread **)_allocator->allocate(_numThreads * sizeof(Thread *));

  if (!_threads) {
    return false;
  }

  for (int i = 0; i < _numThreads; ++i) {
    _threads[i] = 0;
  }

  for (int i = 0; i < _numThreads; ++i) {
    _threads[i] =
        new (_allocator) ThreadPoolWorker(i + 1, _name, &_queue, _logger);

    if (0 == _threads[i]) {
      destroyWorkerThreads();

      return false;
    }
  }

  return true;
}

void ThreadPool::destroyWorkerThreads() {
  if (!_threads) {
    return;
  }

  for (int i = 0; i < _numThreads; ++i) {
    if (0 == _threads[i]) {
      break;
    }

    _threads[i]->~Thread();
    _allocator->deallocate(_threads[i]);
  }

  _allocator->deallocate(_threads);
  _threads = 0;
}

ThreadPoolWorker::ThreadPoolWorker(int workerId, const char *name,
                                   SharedEmbeddedQueue *queue, Logger *logger)
    : _workerId(workerId), _name(name), _queue(queue), _logger(logger) {}

ThreadPoolWorker::~ThreadPoolWorker() {}

void ThreadPoolWorker::run() {
  if (_logger->isLoggable(Logger::Notice)) {
    _logger->log(Logger::Notice, __FILE__, __LINE__, "[%s:%d] starting", _name,
                 _workerId);
  }

  Error error;
  Command *command = 0;
  CleanupHandler *cleanupHandler = 0;
  int errorCount = 0;
  bool cleanup = false;

  while (isRunning() && errorCount < 10) {
    command = (Command *)_queue->pop(&error);

    if (ESB_SHUTDOWN == error) {
      if (_logger->isLoggable(Logger::Debug)) {
        _logger->log(Logger::Debug, __FILE__, __LINE__,
                     "[%s:%d] received shutdown command", _name, _workerId);
      }

      break;
    }

    if (ESB_SUCCESS != error) {
      if (_logger->isLoggable(Logger::Err)) {
        char buffer[100];

        DescribeError(error, buffer, sizeof(buffer));

        _logger->log(Logger::Err, __FILE__, __LINE__,
                     "[%s:%d] cannot pop command from queue: %s", _name,
                     _workerId, buffer);
      }

      ++errorCount;

      continue;
    }

    // todo add performance counters

    if (_logger->isLoggable(Logger::Debug)) {
      _logger->log(Logger::Debug, __FILE__, __LINE__,
                   "[%s:%d] starting command %s", _name, _workerId,
                   command->getName());
    }

    cleanup = command->run(&_isRunning);

    if (_logger->isLoggable(Logger::Debug)) {
      _logger->log(Logger::Debug, __FILE__, __LINE__,
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

  if (_logger->isLoggable(Logger::Notice)) {
    _logger->log(Logger::Notice, __FILE__, __LINE__, "[%s:%d] exiting", _name,
                 _workerId);
  }
}

}  // namespace ESB
