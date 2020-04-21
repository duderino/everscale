#ifndef ES_HTTP_SERVER_H
#include <ESHttpServer.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

namespace ES {

HttpServer::HttpServer(ESB::UInt32 threads, HttpServerHandler &serverHandler,
                       ESB::Allocator &allocator)
    : _threads(0 >= threads ? 1 : threads),
      _state(ES_HTTP_SERVER_IS_DESTROYED),
      _allocator(allocator),
      _serverHandler(serverHandler),
      _multiplexers(),
      _threadPool("HttpServerMultiplexerPool", _threads),
      _rand(),
      _serverCounters() {}

HttpServer::~HttpServer() {
  if (!(_state.get() & ES_HTTP_SERVER_IS_DESTROYED)) {
    destroy();
  }
}

ESB::Error HttpServer::push(HttpServerCommand *command, int idx) {
  if (ES_HTTP_SERVER_IS_STARTED != _state.get()) {
    return ESB_INVALID_STATE;
  }

  if (!command) {
    return ESB_NULL_POINTER;
  }

  if (0 > idx) {
    // both endpoints of the range are inclusive.
    idx = _rand.generate(0, _threads - 1);
  }

  HttpServerMultiplexer *multiplexer =
      (HttpServerMultiplexer *)_multiplexers.index(idx);
  assert(multiplexer);
  ESB::Error error = multiplexer->pushServerCommand(command);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot queue command on multiplexer");
    return error;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServer::addListener(ESB::ListeningTCPSocket &listener) {
  if (ESB::ListeningTCPSocket::SocketState::BOUND != listener.state()) {
    return ESB_INVALID_ARGUMENT;
  }

#ifndef HAVE_SO_REUSEPORT
  ESB::Error error = listener.listen();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot listen on %s", listener.logAddress());
    return error;
  }
#endif

  for (int i = 0; i < _threads; ++i) {
    AddListeningSocketCommand *command =
        new (ESB::SystemAllocator::Instance()) AddListeningSocketCommand(
            listener, ESB::SystemAllocator::Instance().cleanupHandler());
    ESB::Error error = push(command, i);
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "Cannot push add listener command");
      return error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServer::initialize() {
  assert(ES_HTTP_SERVER_IS_DESTROYED == _state.get());
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

  for (ESB::UInt32 i = 0; i < _threads; ++i) {
    ESB::SocketMultiplexer *multiplexer = createMultiplexer();

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
  ESB_LOG_DEBUG("Started");
  return ESB_SUCCESS;
}

ESB::Error HttpServer::stop() {
  assert(ES_HTTP_SERVER_IS_STARTED == _state.get());
  _state.set(ES_HTTP_SERVER_IS_STOPPED);

  ESB_LOG_DEBUG("Stopping");
  _threadPool.stop();
  ESB_LOG_DEBUG("Stopped");

  return ESB_SUCCESS;
}

void HttpServer::destroy() {
  assert(ES_HTTP_SERVER_IS_STOPPED == _state.get());
  _state.set(ES_HTTP_SERVER_IS_DESTROYED);

  for (ESB::ListIterator it = _multiplexers.frontIterator(); !it.isNull();
       it = it.next()) {
    destroyMultiplexer((HttpServerMultiplexer *)it.value());
  }

  _multiplexers.clear();
}

ESB::SocketMultiplexer *HttpServer::createMultiplexer() {
  return new (_allocator)
      HttpServerMultiplexer(ESB::SystemConfig::Instance().socketSoftMax(),
                            _serverHandler, _serverCounters);
}

void HttpServer::destroyMultiplexer(ESB::SocketMultiplexer *multiplexer) {
  multiplexer->~SocketMultiplexer();
  _allocator.deallocate(multiplexer);
}

}  // namespace ES
