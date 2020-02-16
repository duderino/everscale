/** @file ESFEpollMultiplexer.h
 *  @brief A linux epoll-based implementation of ESFSocketMultiplexer
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_EPOLL_MULTIPLEXER_H
#define ESF_EPOLL_MULTIPLEXER_H

#ifndef ESF_SOCKET_MULTIPLEXER_H
#include <ESFSocketMultiplexer.h>
#endif

#ifndef ESF_SHARED_COUNTER_H
#include <ESFSharedCounter.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_EMBEDDED_LIST_H
#include <ESFEmbeddedList.h>
#endif

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

#ifndef ESF_SHARED_COUNTER_H
#include <ESFSharedCounter.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifdef HAVE_SYS_EPOLL_H
#include <sys/epoll.h>
#endif

/** A linux epoll-based implementation of ESFSocketMultiplexer.
 *
 * @ingroup thread
 */
class ESFEpollMultiplexer : public ESFSocketMultiplexer {
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
  ESFEpollMultiplexer(const char *name, int maxSockets, ESFLogger *logger,
                      ESFAllocator *allocator);

  /** Destructor.
   */
  virtual ~ESFEpollMultiplexer();

  /** Return an optional handler that can destroy the multiplexer.
   *
   * @return A handler to destroy the element or NULL if the element should not
   * be destroyed.
   */
  virtual ESFCleanupHandler *getCleanupHandler();

  /** Get the name of the multiplexer.  This name can be used in logging
   * messages, etc.
   *
   * @return The multiplexer's name
   */
  virtual const char *getName() const;

  /** Initialize the multiplexer
   *
   * @return ESF_SUCCESS if successful, another error code otherwise
   */
  virtual ESFError initialize();

  /** Destroy the multiplexer
   *
   */
  virtual void destroy();

  /** Add a new multiplexed socket to the socket multiplexer
   *
   * @param multiplexedSocket The multiplexed socket
   * @return ESF_SUCCESS if successful, ESF_OVERFLOW if the maxSockets limit has
   *  been reached, another error code otherwise.
   */
  virtual ESFError addMultiplexedSocket(
      ESFMultiplexedSocket *multiplexedSocket);

  /** Run the multiplexer's event loop until shutdown.
   *
   * @param isRunning This object will return true as long as the controlling
   * thread isRunning, false when the controlling thread wants to shutdown.
   * @return If true, caller should destroy the command with the CleanupHandler.
   */
  virtual bool run(ESFFlag *isRunning);

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
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  //  Disabled
  ESFEpollMultiplexer(const ESFEpollMultiplexer &);
  ESFEpollMultiplexer &operator=(const ESFEpollMultiplexer &);

  /** Keep socket in epoll and socket list, but possibly change the readiness
   *  events of interest.  This does not modify the _currentSocketCount.
   *
   * @param multiplexedSocket The multiplexedSocket
   */
  ESFError updateMultiplexedSocket(ESFFlag *isRunning,
                                   ESFMultiplexedSocket *multiplexedSocket);

  /** Remove a multiplexed socket form the socket multiplexer
   *
   * @param multiplexedSocket The multiplexed socket to remove
   */
  ESFError removeMultiplexedSocket(ESFFlag *isRunning,
                                   ESFMultiplexedSocket *multiplexedSocket,
                                   bool removeFromList = true);

  /** Periodically check for any idle sockets and delete them.
   *
   * @param isRunning This object will return true as long as the controlling
   * thread isRunning, false when the controlling thread wants to shutdown.
   */
  ESFError checkIdleSockets(ESFFlag *isRunning);

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
  ESFAllocator *_allocator;
  ESFSharedCounter _currentSocketCount;
  ESFEmbeddedList _currentSocketList;
  ESFMutex _lock;
};

#endif /* ! ESF_EPOLL_MULTIPLEXER_H */
