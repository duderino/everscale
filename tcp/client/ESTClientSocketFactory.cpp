#ifndef EST_CLIENT_SOCKET_FACTORY_H
#include <ESTClientSocketFactory.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef EST_CLIENT_SOCKET_H
#include <ESTClientSocket.h>
#endif

#include <errno.h>

namespace EST {

ClientSocketFactory::ClientSocketFactory(
    int maxSockets, PerformanceCounter *successCounter,
    ESB::SocketMultiplexerDispatcher *dispatcher, ESB::Logger *logger)
    : _dispatcher(dispatcher),
      _successCounter(successCounter),
      _logger(logger),
      _fixedAllocator(maxSockets + 10, sizeof(ClientSocket),
                      ESB::SystemAllocator::GetInstance()),
      _sharedAllocator(&_fixedAllocator),
      _cleanupHandler(&_sharedAllocator) {}

ClientSocketFactory::~ClientSocketFactory() {}

ESB::Error ClientSocketFactory::initialize() {
  return _sharedAllocator.initialize();
}

ESB::Error ClientSocketFactory::addNewConnection(
    const ESB::SocketAddress &address) {
  ClientSocket *socket = new (&_sharedAllocator) ClientSocket(
      this, _successCounter, -1, address, &_cleanupHandler, _logger);

  if (!socket) {
    if (_logger->isLoggable(ESB::Logger::Critical)) {
      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[factory] Cannot allocate new client socket");
    }

    return ESB_OUT_OF_MEMORY;
  }

  ESB::Error error = ESB_SUCCESS;

  while (true) {
    error = socket->connect();

    if (EADDRNOTAVAIL == errno) {
      if (_logger->isLoggable(ESB::Logger::Warning)) {
        _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                     "[factory] Spurious EADDRNOTAVAIL on connect");
      }

      continue;
    }

    break;
  }

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[factory] Cannot connect to peer: %s", buffer);
    }

    return error;
  }

  error = _dispatcher->addMultiplexedSocket(socket);

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[factory] Cannot add client socket to multiplexer: %s",
                   buffer);
    }

    return error;
  }

  return ESB_SUCCESS;
}

}  // namespace EST
