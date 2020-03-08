#ifndef ESB_THREAD_POOL_H
#include <ESBThreadPool.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
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

ThreadPool::ThreadPool(const char *name, int threads, Allocator *allocator)
    : _numThreads(threads < MIN_THREADS ? MIN_THREADS : threads),
      _name(name ? name : "ThreadPool"),
      _threads(0),
      _allocator(allocator ? allocator : SystemAllocator::GetInstance()),
      _queue() {}

ThreadPool::~ThreadPool() {}

Error ThreadPool::start() {
  ESB_LOG_NOTICE("ThreadPool '%s' starting", _name);

  if (false == createWorkerThreads()) {
    ESB_LOG_CRITICAL("Cannot start ThreadPool '%s': out of threads", _name);
    return ESB_OUT_OF_MEMORY;
  }

  Error error;

  for (int i = 0; i < _numThreads; ++i) {
    ESB_LOG_DEBUG("Starting worker '%s:%d' starting", _name, i + 1);

    error = _threads[i]->start();

    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "Cannot start worker '%s:%d'", _name,
                             i + 1);
      _numThreads = i;
      stop();
      return error;
    }
  }

  ESB_LOG_NOTICE("ThreadPool '%s' started", _name);
  return ESB_SUCCESS;
}

void ThreadPool::stop() {
  ESB_LOG_NOTICE("ThreadPool '%s' stopping", _name);

  // worker threads will get ESB_SHUTDOWN next time they try to pull an item
  // from the queue
  _queue.stop();

  // stop the worker threads if they are in the middle of running a command
  for (int i = 0; i < _numThreads; ++i) {
    _threads[i]->stop();
  }

  Error error;

  for (int i = 0; i < _numThreads; ++i) {
    ESB_LOG_DEBUG("Waiting for worker '%s:%d'", _name, i + 1);

    error = _threads[i]->join();

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Error joining worker '%s:%d'", _name, i + 1);
    }
  }

  destroyWorkerThreads();
  ESB_LOG_NOTICE("ThreadPool '%s' stopped", _name);
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
    _threads[i] = new (_allocator) ThreadPoolWorker(i + 1, _name, &_queue);

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
                                   SharedEmbeddedQueue *queue)
    : _workerId(workerId), _name(name), _queue(queue) {}

ThreadPoolWorker::~ThreadPoolWorker() {}

void ThreadPoolWorker::run() {
  ESB_LOG_DEBUG("Thread '%s:%d' starting", _name, _workerId);

  Error error;
  Command *command = 0;
  CleanupHandler *cleanupHandler = 0;
  int errorCount = 0;
  bool cleanup = false;

  while (isRunning() && errorCount < 10) {
    command = (Command *)_queue->pop(&error);

    if (ESB_SHUTDOWN == error) {
      ESB_LOG_DEBUG("Thread '%s:%d' shutting down", _name, _workerId);
      break;
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Thread '%s:%d' cannot pop command from queue",
                          _name, _workerId);
      ++errorCount;
      continue;
    }

    ESB_LOG_DEBUG("Thread '%s:%d' starting command '%s'", _name, _workerId,
                  command->getName());
    cleanup = command->run(&_isRunning);
    ESB_LOG_DEBUG("Thread '%s:%d' finished command '%s'", _name, _workerId,
                  command->getName());

    if (cleanup) {
      cleanupHandler = command->getCleanupHandler();
      if (cleanupHandler) {
        cleanupHandler->destroy(command);
      }
    }

    command = 0;
    errorCount = 0;
  }

  ESB_LOG_DEBUG("Thread '%s:%d' exiting", _name, _workerId);
}

}  // namespace ESB
