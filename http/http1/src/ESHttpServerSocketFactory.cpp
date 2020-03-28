#ifndef ES_HTTP_SERVER_SOCKET_FACTORY_H
#include <ESHttpServerSocketFactory.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_FIXED_ALLOCATOR_H
#include <ESBFixedAllocator.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

namespace ES {

HttpServerSocketFactory::HttpServerSocketFactory(HttpServerCounters *counters)
    : _counters(counters),
      _unprotectedAllocator(ESB::SystemConfig::Instance().pageSize(),
                            ESB::SystemConfig::Instance().cacheLineSize()),
      _allocator(_unprotectedAllocator),
      _embeddedList(),
      _mutex(),
      _cleanupHandler(this) {}

ESB::Error HttpServerSocketFactory::initialize() { return ESB_SUCCESS; }

void HttpServerSocketFactory::destroy() {
  HttpServerSocket *socket = (HttpServerSocket *)_embeddedList.removeFirst();

  while (socket) {
    socket->~HttpServerSocket();
    socket = (HttpServerSocket *)_embeddedList.removeFirst();
  }
}

HttpServerSocketFactory::~HttpServerSocketFactory() { destroy(); }

HttpServerSocket *HttpServerSocketFactory::create(
    HttpServerHandler *handler, ESB::TCPSocket::State &state) {
  HttpServerSocket *socket = 0;

  _mutex.writeAcquire();
  socket = (HttpServerSocket *)_embeddedList.removeFirst();
  _mutex.writeRelease();

  if (!socket) {
    socket = new (&_allocator)
        HttpServerSocket(handler, &_cleanupHandler, _counters);
    if (!socket) {
      return 0;
    }
  }

  socket->reset(handler, state);
  return socket;
}

void HttpServerSocketFactory::release(HttpServerSocket *socket) {
  if (!socket) {
    return;
  }

  _mutex.writeAcquire();
  _embeddedList.addFirst(socket);
  _mutex.writeRelease();
}

HttpServerSocketFactory::CleanupHandler::CleanupHandler(
    HttpServerSocketFactory *factory)
    : ESB::CleanupHandler(), _factory(factory) {}

HttpServerSocketFactory::CleanupHandler::~CleanupHandler() {}

void HttpServerSocketFactory::CleanupHandler::destroy(ESB::Object *object) {
  _factory->release((HttpServerSocket *)object);
}

}  // namespace ES
