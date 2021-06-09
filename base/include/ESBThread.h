#ifndef ESB_THREAD_H
#define ESB_THREAD_H

#ifndef ESB_DATE_H
#include <ESBDate.h>
#endif

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#else
#error "pthread.h or equivalent is required"
#endif

namespace ESB {

/** @defgroup thread Thread */

/** Thread should be subclassed by any class that wants to run in its own
 *  thread of control.  Subclasses must define a run template method that this
 *  base class will call in a new thread of control.
 *
 *  @ingroup thread
 */
class Thread {
 public:
#ifdef HAVE_PTHREAD_T
  typedef pthread_t ThreadId;
#else
#error "pthread_t or equivalent is required"
#endif

  /** Default constructor.
   */
  Thread();

  /** Default destructor.
   */
  virtual ~Thread();

  /** Spawns a new thread that will immediately call the run method.
   *
   *  @return ESB_SUCCESS if new thread was spawned, another error code
   * otherwise.
   */
  Error start();

  /** Requests the thread stop.  Threads may choose to ignore this request.
   *
   */
  inline void stop() { _isRunning.set(false); }

  /** Blocks the calling thread until this thread finishes execution.
   *
   *  @return ESB_SUCCESS if join was successful, another error code otherwise.
   */
  Error join();

  /** Get the thread id of this thread.
   *
   *    @return the thread id of this thread.
   */
  ThreadId threadId() const;

  /** Get the current thread id of the calling thread.
   *
   *    @return the thread id of the calling thread.
   */
  static ThreadId CurrentThreadId();

  /** Yield the processor to another thread or process.
   */
  static void Yield();

  /** Put the calling thread to sleep
   *
   *  @param date The amount of time to sleep
   */
  static void Sleep(const Date &date);

  /** Put the calling thread to sleep
   *
   *  @param msec The number of milliseconds to sleep
   */
  static void Sleep(long msec);

 protected:
  /** This is the main function for the new thread.  Subclasses must define
   *    this.
   */
  virtual void run() = 0;

  /** Should be called from a subclass's run method on every iteration to
   *  determine whether it should return from its run method.
   *
   *  @return true if the thread should continue running, false otherwise.
   */
  inline bool isRunning() const { return _isRunning.get(); }

  SharedInt _isRunning;

 private:
  static void *ThreadEntry(void *arg);

  ThreadId _threadId;

  ESB_DEFAULT_FUNCS(Thread);
};

}  // namespace ESB

#endif
