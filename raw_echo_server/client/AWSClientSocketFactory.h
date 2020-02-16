/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_CLIENT_SOCKET_FACTORY_H
#define AWS_CLIENT_SOCKET_FACTORY_H

#ifndef ESF_FIXED_ALLOCATOR_H
#include <ESFFixedAllocator.h>
#endif

#ifndef ESF_SHARED_ALLOCATOR_H
#include <ESFSharedAllocator.h>
#endif

#ifndef ESF_ALLOCATOR_CLEANUP_HANDLER_H
#include <ESFAllocatorCleanupHandler.h>
#endif

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

#ifndef ESF_SOCKET_MULTIPLEXER_DISPATCHER_H
#include <ESFSocketMultiplexerDispatcher.h>
#endif

#ifndef ESF_SOCKET_ADDRESS_H
#include <ESFSocketAddress.h>
#endif

#ifndef AWS_PERFORMANCE_COUNTER_H
#include <AWSPerformanceCounter.h>
#endif

/** A factory that creates new AWSClientSockets and adds them to
 *  a socket multiplexer dispatcher.
 */
class AWSClientSocketFactory {
 public:
  /** Constructor
   *
   * @param maxSockets The maximum number of sockets the factory will have to
   * create
   * @param dispatcher New sockets will be added to this dispatcher
   * @param logger A logger
   */
  AWSClientSocketFactory(int maxSockets, AWSPerformanceCounter *successCounter,
                         ESFSocketMultiplexerDispatcher *dispatcher,
                         ESFLogger *logger);

  /** Destructor.
   */
  virtual ~AWSClientSocketFactory();

  ESFError initialize();

  /** Create a new socket and add it to the multiplexer dispatcher
   *
   * @param address The address the new socket should connect to.
   * @return ESF_SUCCESS if successful, another error code otherwise.
   */
  ESFError addNewConnection(const ESFSocketAddress &address);

 private:
  // Disabled
  AWSClientSocketFactory(const AWSClientSocketFactory &);
  AWSClientSocketFactory &operator=(const AWSClientSocketFactory &);

  ESFSocketMultiplexerDispatcher *_dispatcher;
  AWSPerformanceCounter *_successCounter;
  ESFLogger *_logger;
  ESFFixedAllocator _fixedAllocator;
  ESFSharedAllocator _sharedAllocator;
  ESFAllocatorCleanupHandler _cleanupHandler;
};

#endif
