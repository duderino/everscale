#ifndef ESB_THREAD_POOL_H
#include <ESBThreadPool.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ESB {

#define MIN_THREADS 1

class ThreadPoolWorker : public Thread {
 public:
  ThreadPoolWorker(int workerId, const char *name, SharedEmbeddedQueue *queue);

  virtual ~ThreadPoolWorker();

  virtual void run();

 private:
  // Disabled
  ThreadPoolWorker(const ThreadPoolWorker &worker);
  void operator=(const ThreadPoolWorker &worker);

  int _workerId;
  const char *_name;
  SharedEmbeddedQueue *_queue;
};

ThreadPool::ThreadPool(const char *namePrefix, UInt32 threads, Allocator &allocator)
    : _numThreads(threads < MIN_THREADS ? MIN_THREADS : threads), _threads(0), _allocator(allocator), _queue() {
  snprintf(_name, sizeof(_name), "%s-%s", namePrefix, "pool");
  _name[sizeof(_name) - 1] = 0;
}

ThreadPool::~ThreadPool() {}

Error ThreadPool::start() {
  ESB_LOG_DEBUG("[%s] starting", _name);

  Error error = createWorkerThreads();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "[%s] is out of threads", _name);
    return error;
  }

  for (int i = 0; i < _numThreads; ++i) {
    ESB_LOG_DEBUG("[%s:%d] starting worker", _name, i + 1);

    error = _threads[i]->start();
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "[%s:%d] cannot start worker", _name, i + 1);
      _numThreads = i;
      stop();
      join();
      return error;
    }
  }

  ESB_LOG_NOTICE("[%s] started", _name);
  return ESB_SUCCESS;
}

void ThreadPool::stop() {
  ESB_LOG_DEBUG("[%s] stopping", _name);

  // worker threads will get ESB_SHUTDOWN next time they try to pull an item
  // from the queue
  _queue.stop();

  // stop the worker threads if they are in the middle of running a command
  for (int i = 0; i < _numThreads; ++i) {
    _threads[i]->stop();
  }
}

Error ThreadPool::join() {
  for (int i = 0; i < _numThreads; ++i) {
    ESB_LOG_DEBUG("[%s:%d} joining worker", _name, i + 1);
    Error error = _threads[i]->join();
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "[%s:%d] error joining worker", _name, i + 1);
      return error;
    }
  }

  destroyWorkerThreads();
  ESB_LOG_NOTICE("[%s] stopped", _name);
  return ESB_SUCCESS;
}

Error ThreadPool::createWorkerThreads() {
  Error error = _allocator.allocate(_numThreads * sizeof(Thread *), (void **)&_threads);
  if (ESB_SUCCESS != error) {
    return error;
  }

  for (int i = 0; i < _numThreads; ++i) {
    _threads[i] = NULL;
  }

  for (int i = 0; i < _numThreads; ++i) {
    _threads[i] = new (_allocator) ThreadPoolWorker(i + 1, _name, &_queue);

    if (!_threads[i]) {
      destroyWorkerThreads();
      return ESB_OUT_OF_MEMORY;
    }
  }

  return ESB_SUCCESS;
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
    _allocator.deallocate(_threads[i]);
  }

  _allocator.deallocate(_threads);
  _threads = 0;
}

ThreadPoolWorker::ThreadPoolWorker(int workerId, const char *name, SharedEmbeddedQueue *queue)
    : _workerId(workerId), _name(name), _queue(queue) {}

ThreadPoolWorker::~ThreadPoolWorker() {}

void ThreadPoolWorker::run() {
  ESB_LOG_DEBUG("[%s:%d] worker starting", _name, _workerId);

  Error error;
  Command *command = 0;
  CleanupHandler *cleanupHandler = 0;
  int errorCount = 0;
  bool cleanup = false;

  while (isRunning() && errorCount < 10) {
    command = (Command *)_queue->pop(&error);

    if (ESB_SHUTDOWN == error) {
      ESB_LOG_DEBUG("[%s:%d] worker shutting down", _name, _workerId);
      break;
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "[%s:%d] worker cannot pop command from queue", _name, _workerId);
      ++errorCount;
      continue;
    }

    ESB_LOG_DEBUG("[%s:%d] worker starting command '%s'", _name, _workerId, command->name());
    cleanup = command->run(&_isRunning);
    ESB_LOG_DEBUG("[%s:%d] worker finished command '%s'", _name, _workerId, command->name());

    if (cleanup) {
      cleanupHandler = command->cleanupHandler();
      if (cleanupHandler) {
        cleanupHandler->destroy(command);
      }
    }

    command = 0;
    errorCount = 0;
  }

  ESB_LOG_DEBUG("[%s:%d] worker exiting", _name, _workerId);
}

}  // namespace ESB
