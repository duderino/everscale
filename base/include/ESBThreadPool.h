#ifndef ESB_THREAD_POOL_H
#define ESB_THREAD_POOL_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_THREAD_H
#include <ESBThread.h>
#endif

#ifndef ESB_SHARED_EMBEDDED_QUEUE_H
#include <ESBSharedEmbeddedQueue.h>
#endif

#ifndef ESB_COMMAND_H
#include <ESBCommand.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ESB {

/** A thread pool that executes Commands
 *
 *  @ingroup thread
 */
class ThreadPool {
 public:
  /** Constructor.
   *
   * @param name The name of the thread pool (included in all log messages).
   *  Caller responsible for the strings memory - use a string literal if
   * possible.
   * @param threads The number of threads to start for the thread pool.
   * @param logger An optional logger.  Pass NULL to not log anything.
   * @param allocator Worker threads will be allocated with this
   *  allocator.
   */
  ThreadPool(const char *name, int threads, Logger *logger,
             Allocator *allocator);

  /** Destructor.
   */
  virtual ~ThreadPool();

  /** Start the threads in the thread pool
   *
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error start();

  /** Stop the thread pool immediately.  Each queued command that is
   *  not executed will be passed to a cleanup handler.  Function
   *  will return after all worker threads have exited.
   */
  void stop();

  /** Run a command in the thread pool.  If the thread pool has
   *  not been started, the commands will just sit in an internal
   *  queue until it is started.
   *
   * @param command The command to execute
   * @return ESB_SUCCESS if successful, ESB_SHUTDOWN if stop has
   *  already been called, another error code otherwise.
   */
  inline Error execute(Command *command) { return _queue.push(command); }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

  /** Get the size in bytes used for each worker thread.  This
   *  might help create an appropriate allocator for the
   *  thread pool, or the default system allocator could just be
   *  used.
   *
   * @return The size in bytes for each worker thread.
   */
  static UWord GetWorkerThreadSize();

 private:
  //  Disabled
  ThreadPool(const ThreadPool &);
  ThreadPool &operator=(const ThreadPool &);

  bool createWorkerThreads();
  void destroyWorkerThreads();

  int _numThreads;
  const char *_name;
  Thread **_threads;
  Allocator *_allocator;
  Logger *_logger;

  SharedEmbeddedQueue _queue;
};

}  // namespace ESB

#endif
