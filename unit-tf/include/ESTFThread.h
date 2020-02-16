#ifndef ESTF_THREAD_H
#define ESTF_THREAD_H

#ifndef ESTF_CONFIG_H
#include <ESTFConfig.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

namespace ESTF {

/** Thread should be subclassed by any class that wants to run in its own
 *  thread of control.  Subclasses must define a run template method that this
 *  base class will call in a new thread of control.
 *
 *  @ingroup unit-test
 */
class Thread {
 public:
#ifdef HAVE_PTHREAD_T
  typedef pthread_t ThreadId;
#else
#error "thread type required"
#endif

  /** Default constructor. */
  Thread();

  /** Default destructor. */
  virtual ~Thread();

  /** Spawns a new thread that will immediately call the run method.
   *
   *    @return true if new thread was spawned, false otherwise.
   */
  bool spawn();

  /** Blocks the calling thread until this thread finishes execution.
   *
   *    @return true if join was successful, false otherwise.
   */
  bool join();

  /** Get the thread id of this thread.
   *
   *    @return the thread id of this thread.
   */
  ThreadId getThreadId();

  /** Get the current thread id of the calling thread.
   *
   *    @return the thread id of the calling thread.
   */
  static ThreadId GetThreadId();

  /** Yield the processor to another thread or process.
   */
  static void Yield();

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

 private:
  static void *ThreadEntry(void *arg);

  ThreadId _threadId;
};

}  // namespace ESTF

#endif
