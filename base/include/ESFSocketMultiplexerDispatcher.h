/** @file ESFSocketMultiplexerDispatcher.h
 *  @brief A dispatcher that evenly distributes sockets across multiple socket
 * multiplexers.
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

#ifndef ESF_SOCKET_MULTIPLEXER_DISPATCHER_H
#define ESF_SOCKET_MULTIPLEXER_DISPATCHER_H

#ifndef ESF_SOCKET_MULTIPLEXER_H
#include <ESFSocketMultiplexer.h>
#endif

#ifndef ESF_SOCKET_MULTIPLEXER_FACTORY_H
#include <ESFSocketMultiplexerFactory.h>
#endif

#ifndef ESF_THREAD_POOL_H
#include <ESFThreadPool.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

/** A dispatcher that evenly distributes sockets across multiple socket
 * multiplexers.
 *
 * @ingroup network
 */
class ESFSocketMultiplexerDispatcher {
 public:
  /** Constructor.
   *
   * @param maximumSockets The total number of sockets to be handled by all
   *  the multiplexers.  Each multiplexer will handle
   * maximumSockets/multiplexerCount sockets.
   * @param multiplexerCount The number of multiplexers to create
   * @param factory A factory that will create the multiplexers
   * @param allocator An allocator that will allocate any memory needed by the
   *  dispatcher, multiplexers, or multiplexer factory.
   * @param name The name of the dispatcher.  Will be used in logging messages.
   *  Caller must ensure the memory is valid for the lifetime of this object -
   *  use a string literal if possible.
   * @param logger An optional logger.  Pass NULL to not log anything.
   */
  ESFSocketMultiplexerDispatcher(ESFUInt16 maximumSockets,
                                 ESFUInt16 multiplexerCount,
                                 ESFSocketMultiplexerFactory *factory,
                                 ESFAllocator *allocator, const char *name,
                                 ESFLogger *logger);

  /** Destructor.
   */
  virtual ~ESFSocketMultiplexerDispatcher();

  /** Start the dispatcher
   *
   * @return ESF_SUCCESS if successful, another error code otherwise.
   */
  ESFError start();

  /** Stop the dispatcher.
   */
  void stop();

  /** Add a multiplexed socket to the multiplexer dispatcher.  The dispatcher
   *  will add the socket to the multiplexer handling the fewest number of
   * sockets.
   *
   * @param multiplexedSocket The multiplexed socket to add
   * @return ESF_SUCCESS if successful, ESF_OVERFLOW if the maximum sockets this
   *  multiplexer can handle has been reached, another error code otherwise.
   */
  ESFError addMultiplexedSocket(ESFMultiplexedSocket *multiplexedSocket);

  /** Add a multiplexed socket to a specific multiplexer.
   *
   * @param multiplexerIndex The index of the multiplexer.  This ranges from 0
   * to multiplexerCount - 1, inclusive of endpoints.
   * @param multiplexedSocket The multiplexed socket to add
   * @return ESF_SUCCESS if successful, ESF_OVERFLOW if the maximum sockets this
   *  multiplexer can handle has been reached, another error code otherwise.
   */
  ESFError addMultiplexedSocket(int multiplexerIndex,
                                ESFMultiplexedSocket *multiplexedSocket);

  /** Get the number of sockets this multiplexer is currently handling.
   *
   * @return the number of sockets this multiplexer is currently handling.
   */
  int getCurrentSockets();

  /** Get the maximum number of sockets this multiplexer can handle.
   *
   * @return the maximum number of sockets this multiplexer can handle.
   */
  int getMaximumSockets();

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

  /** Get the maximum number of sockets this process can handle.
   *
   * @return The maxmium number of sockets this process can handle.
   */
  static ESFUInt16 GetMaximumSockets();

 private:
  //  Disabled
  ESFSocketMultiplexerDispatcher(const ESFSocketMultiplexerDispatcher &);
  ESFSocketMultiplexerDispatcher &operator=(
      const ESFSocketMultiplexerDispatcher &);

  ESFError createMultiplexers();

  void destroyMultiplexers();

  ESFUInt16 _maximumSockets;
  ESFUInt16 _multiplexerCount;
  const char *_name;
  ESFLogger *_logger;
  ESFSocketMultiplexerFactory *_factory;
  ESFAllocator *_allocator;
  ESFSocketMultiplexer **_multiplexers;
  ESFThreadPool _threadPool;
};

#endif /* ! ESF_SOCKET_MULTIPLEXER_DISPATCHER_H */
