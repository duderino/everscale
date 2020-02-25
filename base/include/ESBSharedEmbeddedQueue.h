#ifndef ESB_SHARED_EMBEDDED_QUEUE_H
#define ESB_SHARED_EMBEDDED_QUEUE_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifndef ESB_COUNTING_SEMAPHORE_H
#include <ESBCountingSemaphore.h>
#endif

namespace ESB {

/** A producer/consumer queue of EmbeddedListElements
 *
 *  @ingroup collection
 */
class SharedEmbeddedQueue {
 public:
  /** Constructor.
   *
   * @param cleanupHandler Any items in the queue when it is stopped will
   *  be passsed to this handler for destruction.
   */
  SharedEmbeddedQueue();

  /** Destructor.
   */
  virtual ~SharedEmbeddedQueue();

  /** Push an element into the queue
   *
   * @param element The element to insert
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error push(EmbeddedListElement *element);

  /** Pop an element from the queue, blocking until an element
   *  is available if necessary.
   *
   * @param error An optional Error to receive the result.  Will be
   *  set to ESB_SUCCESS if successful, ESB_SHUTDOWN if the queue has been
   *  stopped, another error code otherwise.
   * @return An element or NULL if the operation failed.
   */
  EmbeddedListElement *pop(Error *error = 0);

  /** Stop the queue.  This will do three things.  New pushes will
   *  fail with the error ESB_SHUTDOWN.  Threads blocked on pop calls will
   *  immediately return with ESB_SHUTDOWN.  Items in the queue will be
   *  passed to the cleanup handler and removed.
   */
  void stop();

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The memory for the new object or NULL of the memory allocation
   * failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  //  Disabled
  SharedEmbeddedQueue(const SharedEmbeddedQueue &);
  SharedEmbeddedQueue &operator=(const SharedEmbeddedQueue &);

  bool _isStopped;

#if defined HAVE_PTHREAD_MUTEX_T && defined HAVE_PTHREAD_COND_T
  pthread_mutex_t _mutex;
  pthread_cond_t _signal;
#else
#error "Mutex and condition variable or equivalent is required"
#endif

  EmbeddedList _list;
};

}  // namespace ESB

#endif
