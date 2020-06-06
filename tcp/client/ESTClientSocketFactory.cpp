#ifndef EST_CLIENT_SOCKET_FACTORY_H
#include <ESTClientSocketFactory.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef EST_CLIENT_SOCKET_H
#include <ESTClientSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

namespace EST {

ClientSocketFactory::ClientSocketFactory(int maxSockets, PerformanceCounter *successCounter,
                                         ESB::SocketMultiplexerDispatcher *dispatcher)
    : _dispatcher(dispatcher),
      _successCounter(successCounter),
      _fixedAllocator(maxSockets + 10, sizeof(ClientSocket), ESB::SystemAllocator::GetInstance()),
      _sharedAllocator(&_fixedAllocator),
      _cleanupHandler(&_sharedAllocator) {}

ClientSocketFactory::~ClientSocketFactory() {}

ESB::Error ClientSocketFactory::initialize() { return _sharedAllocator.initialize(); }

ESB::Error ClientSocketFactory::addNewConnection(const ESB::SocketAddress &address) {
  ClientSocket *socket = new (&_sharedAllocator) ClientSocket(this, _successCounter, -1, address, &_cleanupHandler);

  if (!socket) {
    ESB_LOG_ERROR("Cannot allocate new client socket");
    return ESB_OUT_OF_MEMORY;
  }

  ESB::Error error = ESB_SUCCESS;

  for (int i = 0; i < 3; ++i) {
    error = socket->connect();
    if (EADDRNOTAVAIL == errno) {
      continue;
    }
    break;
  }

  if (ESB_SUCCESS != error) {
    if (ESB_INFO_LOGGABLE) {
      char dottedIP[ESB_IPV6_PRESENTATION_SIZE];
      address.getIPAddress(dottedIP, sizeof(dottedIP));
      ESB_LOG_INFO_ERRNO(error, "Cannot connect to %s:%d", dottedIP, address.getPort());
    }
    return error;
  }

  error = _dispatcher->addMultiplexedSocket(socket);

  if (ESB_SUCCESS != error) {
    ESB_LOG_WARNING_ERRNO(error, "Cannot add client socket to multiplexer");
    return error;
  }

  return ESB_SUCCESS;
}

}  // namespace EST
