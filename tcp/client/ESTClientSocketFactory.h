#ifndef EST_CLIENT_SOCKET_FACTORY_H
#define EST_CLIENT_SOCKET_FACTORY_H

#ifndef ESB_FIXED_ALLOCATOR_H
#include <ESBFixedAllocator.h>
#endif

#ifndef ESB_SHARED_ALLOCATOR_H
#include <ESBSharedAllocator.h>
#endif

#ifndef ESB_ALLOCATOR_CLEANUP_HANDLER_H
#include <ESBAllocatorCleanupHandler.h>
#endif

#ifndef ESB_SOCKET_MULTIPLEXER_DISPATCHER_H
#include <ESBSocketMultiplexerDispatcher.h>
#endif

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef EST_PERFORMANCE_COUNTER_H
#include <ESTPerformanceCounter.h>
#endif

namespace EST {

/** A factory that creates new ClientSockets and adds them to
 *  a socket multiplexer dispatcher.
 */
class ClientSocketFactory {
 public:
  /** Constructor
   *
   * @param maxSockets The maximum number of sockets the factory will have to
   * create
   * @param dispatcher New sockets will be added to this dispatcher
   */
  ClientSocketFactory(int maxSockets, PerformanceCounter *successCounter, ESB::SocketMultiplexerDispatcher *dispatcher);

  /** Destructor.
   */
  virtual ~ClientSocketFactory();

  ESB::Error initialize();

  /** Create a new socket and add it to the multiplexer dispatcher
   *
   * @param address The address the new socket should connect to.
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error addNewConnection(const ESB::SocketAddress &address);

 private:
  // Disabled
  ClientSocketFactory(const ClientSocketFactory &);
  ClientSocketFactory &operator=(const ClientSocketFactory &);

  ESB::SocketMultiplexerDispatcher *_dispatcher;
  PerformanceCounter *_successCounter;
  ESB::FixedAllocator _fixedAllocator;
  ESB::SharedAllocator _sharedAllocator;
  ESB::AllocatorCleanupHandler _cleanupHandler;
};

}  // namespace EST

#endif
