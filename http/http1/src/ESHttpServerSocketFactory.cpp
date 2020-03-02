#ifndef ES_HTTP_SERVER_SOCKET_FACTORY_H
#include <ESHttpServerSocketFactory.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_FIXED_ALLOCATOR_H
#include <ESBFixedAllocator.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ES {

HttpServerSocketFactory::HttpServerSocketFactory(HttpServerCounters *counters,
                                                 ESB::Logger *logger)
    : _logger(logger ? logger : ESB::NullLogger::GetInstance()),
      _counters(counters),
      _unprotectedAllocator(ESB_WORD_ALIGN(sizeof(HttpServerSocket)) * 100,
                            ESB::SystemAllocator::GetInstance()),
      _allocator(&_unprotectedAllocator),
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

  _allocator.destroy();
}

HttpServerSocketFactory::~HttpServerSocketFactory() { destroy(); }

HttpServerSocket *HttpServerSocketFactory::create(
    HttpServerHandler *handler, ESB::TCPSocket::AcceptData *acceptData) {
  HttpServerSocket *socket = 0;

  _mutex.writeAcquire();
  socket = (HttpServerSocket *)_embeddedList.removeFirst();
  _mutex.writeRelease();

  if (!socket) {
    socket = new (&_allocator)
        HttpServerSocket(handler, &_cleanupHandler, _logger, _counters);
    if (!socket) {
      return 0;
    }
  }

  socket->reset(handler, acceptData);
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
