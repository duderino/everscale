#ifndef ESB_EPOLL_MULTIPLEXER_H
#define ESB_EPOLL_MULTIPLEXER_H

#ifndef ESB_SOCKET_MULTIPLEXER_H
#include <ESBSocketMultiplexer.h>
#endif

#ifndef ESB_SHARED_COUNTER_H
#include <ESBSharedCounter.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_SHARED_COUNTER_H
#include <ESBSharedCounter.h>
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
   * @param name The name of the multiplexer to be used in log messages.  Caller
   * is responsible for the strings memory - use a string literal if possible.
   * @param maxSockets The maximum number of sockets the multiplexer will
   *  handle. When this limit is hit, the multiplexer will stop accepting
   *  new connections from any listening sockets and reject any application
   *  requests to add new sockets.
   * @param logger An optional logger.  Pass NULL to not log anything.
   * @param allocator Internal storage will be allocated using this allocator.
   */
  EpollMultiplexer(const char *name, int maxSockets, Logger *logger,
                   Allocator *allocator);

  /** Destructor.
   */
  virtual ~EpollMultiplexer();

  /** Return an optional handler that can destroy the multiplexer.
   *
   * @return A handler to destroy the element or NULL if the element should not
   * be destroyed.
   */
  virtual CleanupHandler *getCleanupHandler();

  /** Get the name of the multiplexer.  This name can be used in logging
   * messages, etc.
   *
   * @return The multiplexer's name
   */
  virtual const char *getName() const;

  /** Initialize the multiplexer
   *
   * @return ESB_SUCCESS if successful, another error code otherwise
   */
  virtual Error initialize();

  /** Destroy the multiplexer
   *
   */
  virtual void destroy();

  /** Add a new multiplexed socket to the socket multiplexer
   *
   * @param multiplexedSocket The multiplexed socket
   * @return ESB_SUCCESS if successful, ESB_OVERFLOW if the maxSockets limit has
   *  been reached, another error code otherwise.
   */
  virtual Error addMultiplexedSocket(MultiplexedSocket *multiplexedSocket);

  /** Run the multiplexer's event loop until shutdown.
   *
   * @param isRunning This object will return true as long as the controlling
   * thread isRunning, false when the controlling thread wants to shutdown.
   * @return If true, caller should destroy the command with the CleanupHandler.
   */
  virtual bool run(Flag *isRunning);

  /** Get the number of sockets this multiplexer is currently handling.
   *
   * @return the number of sockets this multiplexer is currently handling.
   */
  virtual int getCurrentSockets();

  /** Get the maximum number of sockets this multiplexer can handle.
   *
   * @return the maximum number of sockets this multiplexer can handle.
   */
  virtual int getMaximumSockets();

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  //  Disabled
  EpollMultiplexer(const EpollMultiplexer &);
  EpollMultiplexer &operator=(const EpollMultiplexer &);

  /** Keep socket in epoll and socket list, but possibly change the readiness
   *  events of interest.  This does not modify the _currentSocketCount.
   *
   * @param multiplexedSocket The multiplexedSocket
   */
  Error updateMultiplexedSocket(Flag *isRunning,
                                MultiplexedSocket *multiplexedSocket);

  /** Remove a multiplexed socket form the socket multiplexer
   *
   * @param multiplexedSocket The multiplexed socket to remove
   */
  Error removeMultiplexedSocket(Flag *isRunning,
                                MultiplexedSocket *multiplexedSocket,
                                bool removeFromList = true);

  /** Periodically check for any idle sockets and delete them.
   *
   * @param isRunning This object will return true as long as the controlling
   * thread isRunning, false when the controlling thread wants to shutdown.
   */
  Error checkIdleSockets(Flag *isRunning);

  int _epollDescriptor;
  int _maxSockets;
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
  Allocator *_allocator;
  SharedCounter _currentSocketCount;
  EmbeddedList _currentSocketList;
  Mutex _lock;
};

}  // namespace ESB

#endif
