#ifndef ESB_SHARED_QUEUE_H
#define ESB_SHARED_QUEUE_H

#ifndef ESB_LIST_H
#include <ESBList.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ESB {

/** SharedQueue is a self-synchronizing queue that handles many producer
 *  threads and many consumer threads.  Consumer threads can block until an
 *  item is added to the queue or return immediately.  Producer threads can
 *  block until there is room in the queue to add an item or return
 *  immediately.
 *
 *  @ingroup collection
 */
class SharedQueue {
 public:
  /** Constructor.
   *
   *  @param allocator The allocator that the queue will use to create its
   *      internal nodes.
   *  @param limit The maximum number of items that can be inserted into the
   *      queue.  Set to 0 for unlimited.  Queues that have no limit will
   *      grow as long as their allocator continue to return memory.
   *  @see GetAllocationSize to determine how much memory the list will
   *      allocate for every internal node it creates.  This is useful for
   *      constructing fixed length allocators.
   */
  SharedQueue(UInt32 limit, Allocator &allocator = SystemAllocator::Instance());

  /** Destructor. */
  virtual ~SharedQueue();

  /** Insert an item into the queue.  O(1).  Block the caller until there
   *  is sufficient room to add a new item.
   *
   *  @param item The item to insert.  Cannot be NULL.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error push(void *item);

  /** Insert an item into the queue.  O(1).  Return immediately if there
   *  is not sufficient room to add a new item.
   *
   *  @param item The item to insert.  Cannot be NULL.
   *  @return ESB_SUCCESS if successful, ESB_AGAIN if there is insufficient
   *      room, another error code otherwise.
   */
  Error tryPush(void *item);

  /** Retrieve an item from the queue.  O(1).  Block the caller until an item
   *  is added to queue.
   *
   *  @param item An item from the queue.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error pop(void **item);

  /** Retrieve an item from the queue.  O(1).  Return immediately if an item
   *  is not available.
   *
   *  @param item An item from the queue
   *  @return ESB_SUCCES if successful, ESB_AGAIN if no items are available,
   *      another error code otherwise.
   */
  Error tryPop(void **item);

  /** Remove all items from the queue.  O(n).  This will only deallocate
   *  memory used by the list's internal nodes.
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error clear();

  /** Get the current size of the queue.  O(1).
   *
   *  @param size The current size of the queue
   *  @return ESB_SUCCESS if successful, another error code otherwise
   */
  Error size(UInt32 *size);

  /** Get the size in bytes of the queue's interal nodes.  This is the amount
   *  of memory that the queue will request from the allocator for every node
   *  it creates.
   *
   *  @return The size in bytes of the queue's internal nodes.
   */
  static Size AllocationSize();

 private:
#if defined HAVE_PTHREAD_MUTEX_T && defined HAVE_PTHREAD_COND_T
  class Synchronizer {
   public:
    /** Default constructor */
    Synchronizer();

    /** Destructor */
    ~Synchronizer();

    pthread_mutex_t _mutex;
    pthread_cond_t _consumerSignal;
    pthread_cond_t _producerSignal;
    UInt8 _magic;
  };
#else
#error "Mutex and condition variable or equivalent is required"
#endif

  UInt32 _limit;
  Synchronizer _lock;
  List _list;

  ESB_DEFAULT_FUNCS(SharedQueue);
};

}  // namespace ESB

#endif
