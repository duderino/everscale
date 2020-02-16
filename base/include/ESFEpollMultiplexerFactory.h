/** @file ESFEpollMultiplexerFactory.h
 *  @brief A factory that creates epoll socket multiplexers
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

#ifndef ESF_EPOLL_MULTIPLEXER_FACTORY_H
#define ESF_EPOLL_MULTIPLEXER_FACTORY_H

#ifndef ESF_SOCKET_MULTIPLEXER_FACTORY_H
#include <ESFSocketMultiplexerFactory.h>
#endif

#ifndef ESF_EPOLL_MULTIPLEXER_H
#include <ESFEpollMultiplexer.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

/** A factory that creates epoll socket multiplexers
 *
 * @ingroup network
 */
class ESFEpollMultiplexerFactory : public ESFSocketMultiplexerFactory {
 public:
  /** Constructor.
   *
   * @param name The name of the created multiplexer to be used in log messages.
   * Caller is responsible for the strings memory - use a string literal if
   * possible.
   * @param logger An optional logger.  Pass NULL to not log anything.
   * @param allocator epoll socket multiplexers will be allocated using this
   * allocator.
   */
  ESFEpollMultiplexerFactory(const char *name, ESFLogger *logger,
                             ESFAllocator *allocator);

  /** Destructor.
   */
  virtual ~ESFEpollMultiplexerFactory();

  /** Create a new socket multiplexer
   *
   * @param maxSockets The maxium number of sockets the multiplexer should
   * handle.
   * @return a new socket multiplexer or NULL if it could not be created
   */
  virtual ESFSocketMultiplexer *create(int maxSockets);

  /** Destroy a socket multiplexer
   *
   * @param multiplexer The socket multiplexer to destroy.
   */
  virtual void destroy(ESFSocketMultiplexer *multiplexer);

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
  ESFEpollMultiplexerFactory(const ESFEpollMultiplexerFactory &);
  ESFEpollMultiplexerFactory &operator=(const ESFEpollMultiplexerFactory &);

  const char *_name;
  ESFLogger *_logger;
  ESFAllocator *_allocator;
};

#endif /* ! ESF_EPOLL_MULTIPLEXER_FACTORY_H */
