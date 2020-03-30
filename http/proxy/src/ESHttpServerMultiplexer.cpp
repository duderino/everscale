#ifndef ES_HTTP_SERVER_MULTIPLEXER_H
#include <ESHttpServerMultiplexer.h>
#endif

namespace ES {

HttpServerMultiplexer::HttpServerMultiplexer(
    ESB::UInt32 maxSockets, ESB::ListeningTCPSocket &listeningSocket,
    HttpServerHandler &serverHandler, HttpServerCounters &serverCounters,
    ESB::Allocator &allocator)
    : HttpMultiplexer(maxSockets, allocator),
      _serverSocketFactory(serverHandler, serverCounters, _factoryAllocator),
      _listeningSocket(serverHandler, listeningSocket, _serverSocketFactory,
                       serverCounters) {}

HttpServerMultiplexer::~HttpServerMultiplexer() { destroy(); }

ESB::Error HttpServerMultiplexer::initialize() {
  return _epollMultiplexer.initialize();
}

bool HttpServerMultiplexer::run(ESB::SharedInt *isRunning) {
  ESB::Error error = addMultiplexedSocket(&_listeningSocket);

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot add listener to multiplexer");
    return false;
  }

  return _epollMultiplexer.run(isRunning);
}

void HttpServerMultiplexer::destroy() { _epollMultiplexer.destroy(); }

const char *HttpServerMultiplexer::name() const {
  return _epollMultiplexer.name();
}

ESB::CleanupHandler *HttpServerMultiplexer::cleanupHandler() { return NULL; }

}  // namespace ES
