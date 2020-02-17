#ifndef ESB_SOCKET_MULTIPLEXER_DISPATCHER_H
#define ESB_SOCKET_MULTIPLEXER_DISPATCHER_H

#ifndef ESB_SOCKET_MULTIPLEXER_H
#include <ESBSocketMultiplexer.h>
#endif

#ifndef ESB_SOCKET_MULTIPLEXER_FACTORY_H
#include <ESBSocketMultiplexerFactory.h>
#endif

#ifndef ESB_THREAD_POOL_H
#include <ESBThreadPool.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ESB {

/** A dispatcher that evenly distributes sockets across multiple socket
 * multiplexers.
 *
 * @ingroup network
 */
class SocketMultiplexerDispatcher {
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
  SocketMultiplexerDispatcher(UInt16 maximumSockets, UInt16 multiplexerCount,
                              SocketMultiplexerFactory *factory,
                              Allocator *allocator, const char *name,
                              Logger *logger);

  /** Destructor.
   */
  virtual ~SocketMultiplexerDispatcher();

  /** Start the dispatcher
   *
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error start();

  /** Stop the dispatcher.
   */
  void stop();

  /** Add a multiplexed socket to the multiplexer dispatcher.  The dispatcher
   *  will add the socket to the multiplexer handling the fewest number of
   * sockets.
   *
   * @param multiplexedSocket The multiplexed socket to add
   * @return ESB_SUCCESS if successful, ESB_OVERFLOW if the maximum sockets this
   *  multiplexer can handle has been reached, another error code otherwise.
   */
  Error addMultiplexedSocket(MultiplexedSocket *multiplexedSocket);

  /** Add a multiplexed socket to a specific multiplexer.
   *
   * @param multiplexerIndex The index of the multiplexer.  This ranges from 0
   * to multiplexerCount - 1, inclusive of endpoints.
   * @param multiplexedSocket The multiplexed socket to add
   * @return ESB_SUCCESS if successful, ESB_OVERFLOW if the maximum sockets this
   *  multiplexer can handle has been reached, another error code otherwise.
   */
  Error addMultiplexedSocket(int multiplexerIndex,
                             MultiplexedSocket *multiplexedSocket);

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
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

  /** Get the maximum number of sockets this process can handle.
   *
   * @return The maxmium number of sockets this process can handle.
   */
  static UInt16 GetMaximumSockets();

 private:
  //  Disabled
  SocketMultiplexerDispatcher(const SocketMultiplexerDispatcher &);
  SocketMultiplexerDispatcher &operator=(const SocketMultiplexerDispatcher &);

  Error createMultiplexers();

  void destroyMultiplexers();

  UInt16 _maximumSockets;
  UInt16 _multiplexerCount;
  const char *_name;
  Logger *_logger;
  SocketMultiplexerFactory *_factory;
  Allocator *_allocator;
  SocketMultiplexer **_multiplexers;
  ThreadPool _threadPool;
};

}  // namespace ESB

#endif
