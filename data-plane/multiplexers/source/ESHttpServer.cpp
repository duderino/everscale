#ifndef ES_HTTP_SERVER_H
#include <ESHttpServer.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ES_HTTP_PROXY_MULTIPLEXER_H
#include <ESHttpProxyMultiplexer.h>
#endif

#ifndef ES_HTTP_CONFIG_H
#include <ESHttpConfig.h>
#endif

namespace ES {

HttpServer::HttpServer(const char *namePrefix, ESB::UInt32 threads, ESB::UInt32 idleTimeoutMsec,
                       HttpServerHandler &serverHandler, ESB::Allocator &allocator)
    : _threads(0 >= threads ? 1 : threads),
      _idleTimeoutMsec(idleTimeoutMsec),
      _state(ES_HTTP_SERVER_IS_DESTROYED),
      _allocator(allocator),
      _serverHandler(serverHandler),
      _multiplexers(),
      _threadPool(namePrefix, _threads),
      _rand(),
      _serverContextIndex(HttpConfig::Instance().tlsContextBuckets(), HttpConfig::Instance().tlsContextLocks(),
                          _allocator),
      _serverCounters() {
  strncpy(_name, namePrefix, sizeof(_name));
  _name[sizeof(_name) - 1] = 0;
}

HttpServer::~HttpServer() {
  if (_state.get() & ES_HTTP_SERVER_IS_DESTROYED) {
    return;
  }

  destroy();
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

  HttpProxyMultiplexer *multiplexer = (HttpProxyMultiplexer *)_multiplexers.index(idx);
  assert(multiplexer);
  ESB::Error error = multiplexer->pushServerCommand(command);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "[%s] cannot queue command on multiplexer", _name);
    return error;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServer::addListener(ESB::ListeningSocket &listener) {
  if (ESB::ListeningSocket::SocketState::BOUND != listener.state()) {
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
    AddListeningSocketCommand *command = new (ESB::SystemAllocator::Instance())
        AddListeningSocketCommand(listener, ESB::SystemAllocator::Instance().cleanupHandler());
    ESB::Error error = push(command, i);
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "[%s] cannot push add listener command", _name);
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
    ESB_LOG_CRITICAL_ERRNO(error, "[%s] cannot start multiplexer threads", _name);
    return error;
  }

  for (ESB::UInt32 i = 0; i < _threads; ++i) {
    ESB::SocketMultiplexer *multiplexer = createMultiplexer();

    if (!multiplexer) {
      ESB_LOG_CRITICAL_ERRNO(ESB_OUT_OF_MEMORY, "[%s] cannot initialize multiplexer", _name);
      return ESB_OUT_OF_MEMORY;
    }

    error = _threadPool.execute(multiplexer);

    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "[%s] cannot schedule multiplexer on thread", _name);
      return error;
    }

    error = _multiplexers.pushBack(multiplexer);

    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "[%s] cannot add multiplexer to multiplexers", _name);
      return error;
    }
  }

  _state.set(ES_HTTP_SERVER_IS_STARTED);
  ESB_LOG_DEBUG("[%s] started", _name);
  return ESB_SUCCESS;
}

void HttpServer::stop() {
  assert(ES_HTTP_SERVER_IS_STARTED == _state.get());
  ESB_LOG_DEBUG("[%s] stopping", _name);
  _threadPool.stop();
  _state.set(ES_HTTP_SERVER_IS_STOPPED);
}

ESB::Error HttpServer::join() {
  assert(ES_HTTP_SERVER_IS_STOPPED == _state.get());
  ESB::Error error = _threadPool.join();
  if (ESB_SUCCESS != error) {
    ESB_LOG_WARNING_ERRNO(error, "[%s] cannot join thread pool", _name);
    return error;
  }
  _state.set(ES_HTTP_SERVER_IS_JOINED);
  ESB_LOG_DEBUG("[%s] stopped", _name);
  return ESB_SUCCESS;
}

void HttpServer::destroy() {
  assert(ES_HTTP_SERVER_IS_JOINED == _state.get());
  _state.set(ES_HTTP_SERVER_IS_DESTROYED);

  for (ESB::ListIterator it = _multiplexers.frontIterator(); !it.isNull(); it = it.next()) {
    destroyMultiplexer((HttpProxyMultiplexer *)it.value());
  }

  _multiplexers.clear();
}

ESB::SocketMultiplexer *HttpServer::createMultiplexer() {
  return new (_allocator) HttpProxyMultiplexer(_name, ESB::SystemConfig::Instance().socketSoftMax(), _idleTimeoutMsec,
                                               _serverHandler, _serverCounters, _serverContextIndex);
}

void HttpServer::destroyMultiplexer(ESB::SocketMultiplexer *multiplexer) {
  multiplexer->~SocketMultiplexer();
  _allocator.deallocate(multiplexer);
}

}  // namespace ES
