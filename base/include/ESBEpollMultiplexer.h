#ifndef ESB_EPOLL_MULTIPLEXER_H
#define ESB_EPOLL_MULTIPLEXER_H

#ifndef ESB_SOCKET_MULTIPLEXER_H
#include <ESBSocketMultiplexer.h>
#endif

#ifndef ESB_SHARED_INT_H
#include <ESBSharedCounter.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_NULL_LOCK_H
#include <ESBNullLock.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifdef HAVE_SYS_EPOLL_H
#include <sys/epoll.h>
#endif

namespace ESB {

/** A linux epoll-based implementation of SocketMultiplexer.
 *
 * @ingroup thread
 */
class EpollMultiplexer : public SocketMultiplexer {
 public:
  /** Constructor.
   *
   * @param maxSockets The maximum number of sockets the multiplexer will
   *  handle. When this limit is hit, the multiplexer will stop accepting
   *  new connections from any listening sockets and reject any application
   *  requests to add new sockets.
   * @param allocator Internal storage will be allocated using this allocator.
   */
  EpollMultiplexer(UInt32 maxSockets,
                   Allocator &allocator = SystemAllocator::Instance(),
                   Lockable &lock = NullLock::Instance());

  /** Destructor.
   */
  virtual ~EpollMultiplexer();

  /** Return an optional handler that can destroy the multiplexer.
   *
   * @return A handler to destroy the element or NULL if the element should not
   * be destroyed.
   */
  virtual CleanupHandler *cleanupHandler();

  /** Get the name of the multiplexer.  This name can be used in logging
   * messages, etc.
   *
   * @return The multiplexer's name
   */
  virtual const char *name() const { return "multiplexer"; }

  /** Add a new multiplexed socket to the socket multiplexer
   *
   * @param socket The multiplexed socket
   * @return ESB_SUCCESS if successful, ESB_OVERFLOW if the maxSockets limit has
   *  been reached, another error code otherwise.
   */
  virtual Error addMultiplexedSocket(MultiplexedSocket *socket);

  /** Run the multiplexer's event loop until shutdown.
   *
   * @return If true, caller should destroy the command with the CleanupHandler.
   */
  virtual bool run(SharedInt *isRunning);

  /** Get the number of sockets this multiplexer is currently handling.
   *
   * @return the number of sockets this multiplexer is currently handling.
   */
  virtual int currentSockets() const;

  /** Get the maximum number of sockets this multiplexer can handle.
   *
   * @return the maximum number of sockets this multiplexer can handle.
   */
  virtual int maximumSockets() const;

  /** Determine whether this multiplexer has been shutdown.
   *
   * @return true if the multiplexer should still run, false if it has been
   *   told to shutdown.
   */
  virtual bool isRunning() const;

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 private:
  //  Disabled
  EpollMultiplexer(const EpollMultiplexer &);
  EpollMultiplexer &operator=(const EpollMultiplexer &);

  /** Destroy the multiplexer
   *
   */
  void destroy();

  /** Keep socket in epoll and socket list, but possibly change the readiness
   *  events of interest.  This does not modify the _currentSocketCount.
   *
   * @param socket The multiplexedSocket
   */
  Error updateMultiplexedSocket(MultiplexedSocket *socket);

  /** Remove a multiplexed socket form the socket multiplexer
   *
   * @param socket The multiplexed socket to remove
   */
  Error removeMultiplexedSocket(MultiplexedSocket *socket,
                                bool removeFromList = true);

  /** Periodically check for any idle sockets and delete them.
   *
   * @param isRunning This object will return true as long as the controlling
   * thread isRunning, false when the controlling thread wants to shutdown.
   */
  Error checkIdleSockets(SharedInt *isRunning);

  struct DescriptorState {
    int _interests;
  };

  int _epollDescriptor;
  UInt32 _maxSockets;
#ifdef HAVE_TIME_T
  time_t _lastIdleCheckSec;
#else
#error "time_t or equilvalent is required"
#endif
#ifdef HAVE_STRUCT_EPOLL_EVENT
  struct epoll_event *_events;
#else
#error "struct epoll_event is required"
#endif
  struct DescriptorState *_eventCache;
  ESB::SharedInt *_isRunning;
  Allocator &_allocator;
  ESB::Lockable &_lock;
  SharedInt _currentSocketCount;
  EmbeddedList _currentSocketList;
};

}  // namespace ESB

#endif
