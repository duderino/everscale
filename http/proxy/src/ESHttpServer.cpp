#ifndef ES_HTTP_SERVER_H
#include <ESHttpServer.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

namespace ES {

HttpServer::HttpServer(ESB::UInt32 threads, ESB::UInt16 port,
                       HttpServerHandler &serverHandler,
                       ESB::Allocator &allocator)
    : _threads(0 >= threads ? 1 : threads),
      _state(ES_HTTP_SERVER_IS_DESTROYED),
      _allocator(allocator),
      _serverHandler(serverHandler),
      _multiplexers(),
      _threadPool("HttpServerMultiplexerPool", _threads),
      _listeningSocket(port, ESB_UINT16_MAX, false),
      _serverCounters() {}

HttpServer::~HttpServer() {
  if (!(_state.get() & ES_HTTP_SERVER_IS_DESTROYED)) {
    destroy();
  }
}

ESB::Error HttpServer::initialize() {
  assert(ES_HTTP_SERVER_IS_DESTROYED == _state.get());

  ESB::Error error = _listeningSocket.bind();

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot bind to port %d",
                           _listeningSocket.listeningAddress().port());
    return error;
  }

  error = _listeningSocket.listen();

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot listen on port %d",
                           _listeningSocket.listeningAddress().port());
    return error;
  }

  _state.set(ES_HTTP_SERVER_IS_INITIALIZED);
  return ESB_SUCCESS;
}

ESB::Error HttpServer::start() {
  assert(ES_HTTP_SERVER_IS_INITIALIZED == _state.get());

  ESB::Error error = _threadPool.start();

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot start multiplexer threads");
    return error;
  }

  ESB::UInt32 maxSockets = ESB::SystemConfig::Instance().socketSoftMax();
  ESB_LOG_NOTICE("Maximum sockets %u", maxSockets);

  for (ESB::UInt32 i = 0; i < _threads; ++i) {
    ESB::SocketMultiplexer *multiplexer = new (_allocator)
        HttpServerMultiplexer(maxSockets, _listeningSocket, _serverHandler,
                              _serverCounters, _allocator);

    if (!multiplexer) {
      ESB_LOG_CRITICAL_ERRNO(ESB_OUT_OF_MEMORY,
                             "Cannot initialize multiplexer");
      return ESB_OUT_OF_MEMORY;
    }

    error = _threadPool.execute(multiplexer);

    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "Cannot schedule multiplexer on thread");
      return error;
    }

    error = _multiplexers.pushBack(multiplexer);

    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "Cannot add multiplexer to multiplexers");
      return error;
    }
  }

  _state.set(ES_HTTP_SERVER_IS_STARTED);
  ESB_LOG_NOTICE("Started");
  return ESB_SUCCESS;
}

ESB::Error HttpServer::stop() {
  assert(ES_HTTP_SERVER_IS_STARTED == _state.get());
  _state.set(ES_HTTP_SERVER_IS_STOPPED);

  ESB_LOG_NOTICE("Stopping");
  _threadPool.stop();
  ESB_LOG_NOTICE("Stopped");

  return ESB_SUCCESS;
}

void HttpServer::destroy() {
  assert(ES_HTTP_SERVER_IS_STOPPED == _state.get());
  _state.set(ES_HTTP_SERVER_IS_DESTROYED);

  for (ESB::ListIterator it = _multiplexers.frontIterator(); !it.isNull();
       it = it.next()) {
    HttpServerMultiplexer *multiplexer = (HttpServerMultiplexer *)it.value();
    multiplexer->~HttpServerMultiplexer();
    _allocator.deallocate(multiplexer);
  }

  _multiplexers.clear();
}

}  // namespace ES
