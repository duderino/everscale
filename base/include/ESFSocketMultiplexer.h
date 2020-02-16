/** @file ESFSocketMultiplexer.h
 *  @brief A command that delegates i/o readiness events to multiple
 * ESFMultiplexedSockets
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

#ifndef ESF_SOCKET_MULTIPLEXER_H
#define ESF_SOCKET_MULTIPLEXER_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ERROR_H
#include <ESFError.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

#ifndef ESF_COMMAND_H
#include <ESFCommand.h>
#endif

#ifndef ESF_MULTIPLEXED_SOCKET_H
#include <ESFMultiplexedSocket.h>
#endif

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

/** A command that delegates i/o readiness events to multiple
 * ESFMultiplexedSockets.  The command can be run in the current thread of
 * control or run by an ESFCommandThread or even in an ESFThreadPool.
 *
 * @ingroup network
 */
class ESFSocketMultiplexer : public ESFCommand {
 public:
  /** Constructor.
   *
   * @param name The name of the multiplexer to be used in log messages.  Caller
   * is responsible for the strings memory - use a string literal if possible.
   * @param logger An optional logger.  Pass NULL to not log anything.
   */
  ESFSocketMultiplexer(const char *name, ESFLogger *logger);

  /** Destructor.
   */
  virtual ~ESFSocketMultiplexer();

  /** Initialize the multiplexer
   *
   * @return ESF_SUCCESS if successful, another error code otherwise
   */
  virtual ESFError initialize() = 0;

  /** Destroy the multiplexer
   *
   */
  virtual void destroy() = 0;

  /** Add a new multiplexed socket to the socket multiplexer
   *
   * @param multiplexedSocket The multiplexed socket to add
   * @return ESF_SUCCESS if successful, ESF_OVERFLOW if the maximum sockets this
   *  multiplexer can handle has been reached, another error code otherwise.
   */
  virtual ESFError addMultiplexedSocket(
      ESFMultiplexedSocket *multiplexedSocket) = 0;

  /** Get the number of sockets this multiplexer is currently handling.
   *
   * @return the number of sockets this multiplexer is currently handling.
   */
  virtual int getCurrentSockets() = 0;

  /** Get the maximum number of sockets this multiplexer can handle.
   *
   * @return the maximum number of sockets this multiplexer can handle.
   */
  virtual int getMaximumSockets() = 0;

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

 protected:
  const char *_name;
  ESFLogger *_logger;

 private:
  //  Disabled
  ESFSocketMultiplexer(const ESFSocketMultiplexer &);
  ESFSocketMultiplexer &operator=(const ESFSocketMultiplexer &);
};

#endif /* ! ESF_SOCKET_MULTIPLEXER_H */
