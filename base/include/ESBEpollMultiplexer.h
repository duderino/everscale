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

#ifndef ESB_FLAT_TIMING_WHEEL_H
#include <ESBFlatTimingWheel.h>
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
  EpollMultiplexer(const char *namePrefix, UInt32 idleTimeoutMsec, UInt32 maxSockets,
                   Allocator &allocator = SystemAllocator::Instance());

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
  virtual const char *name() const;

  /** Add a new multiplexed socket to the socket multiplexer
   *
   * @param socket The multiplexed socket
   * @return ESB_SUCCESS if successful, ESB_OVERFLOW if the maxSockets limit has
   *  been reached, another error code otherwise.
   */
  virtual Error addMultiplexedSocket(MultiplexedSocket *socket);

  /** Keep socket in epoll and socket list, but possibly change the readiness
   *  events of interest.  This does not modify the _activeSocketCount.
   *
   * @param socket The multiplexedSocket
   */
  virtual Error updateMultiplexedSocket(MultiplexedSocket *socket);

  /** Remove a multiplexed socket from the socket multiplexer
   *
   * @param socket The multiplexed socket to remove
   */
  virtual Error removeMultiplexedSocket(MultiplexedSocket *socket);

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
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  /** Destroy the multiplexer
   *
   */
  void destroy();

  /** Remove a multiplexed socket form the socket multiplexer
   *
   * @param socket The multiplexed socket to remove
   */
  Error removeMultiplexedSocketNoLock(MultiplexedSocket *socket);

  /** Periodically check for any idle sockets and close them
   */
  void checkIdleSockets();

  struct DescriptorState {
    int _interests;
  };

  int _epollDescriptor;
  const UInt32 _idleTimeoutMsec;
  const UInt32 _maxSockets;
#ifdef HAVE_STRUCT_EPOLL_EVENT
  struct epoll_event *_events;
#else
#error "struct epoll_event is required"
#endif
  struct DescriptorState *_eventCache;
  ESB::SharedInt *_isRunning;
  Allocator &_allocator;
  SharedInt _activeSocketCount;
  EmbeddedList _activeSockets;
  EmbeddedList _deadSockets;
  FlatTimingWheel _timingWheel;
  char _namePrefix[ESB_NAME_PREFIX_SIZE];

  ESB_DISABLE_AUTO_COPY(EpollMultiplexer);
};

}  // namespace ESB

#endif
