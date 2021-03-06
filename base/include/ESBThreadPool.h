#ifndef ESB_THREAD_POOL_H
#define ESB_THREAD_POOL_H

#ifndef ESB_THREAD_H
#include <ESBThread.h>
#endif

#ifndef ESB_SHARED_EMBEDDED_QUEUE_H
#include <ESBSharedEmbeddedQueue.h>
#endif

#ifndef ESB_COMMAND_H
#include <ESBCommand.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
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
   * @param namePrefix The name of the thread pool (included in all log messages).
   *  Caller responsible for the strings memory - use a string literal if
   * possible.
   * @param threads The number of threads to start for the thread pool.
   * @param allocator Worker threads will be allocated with this
   *  allocator.
   */
  ThreadPool(const char *namePrefix, UInt32 threads, Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~ThreadPool();

  /** Start the threads in the thread pool
   *
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error start();

  /** Stop the thread pool immediately but do not wait for all threads in the threadpool to exit.
   */
  void stop();

  /**
   * Wait for all threads in the thread pool to exit.  This should be called after stop().
   *
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error join();

  /** Run a command in the thread pool.  If the thread pool has
   *  not been started, the commands will just sit in an internal
   *  queue until it is started.
   *
   * @param command The command to execute
   * @return ESB_SUCCESS if successful, ESB_SHUTDOWN if stop has
   *  already been called, another error code otherwise.
   */
  inline Error execute(Command *command) { return _queue.push(command); }

 private:
  Error createWorkerThreads();
  void destroyWorkerThreads();

  UInt32 _numThreads;
  Thread **_threads;
  Allocator &_allocator;
  SharedEmbeddedQueue _queue;
  char _name[ESB_NAME_PREFIX_SIZE + 5];

  ESB_DEFAULT_FUNCS(ThreadPool);
};

}  // namespace ESB

#endif
