#ifndef ES_HTTP_CLIENT_H
#include <ESHttpClient.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

namespace ES {

HttpClient::HttpClient(ESB::UInt32 threads, ESB::UInt32 connections,
                       HttpSeedTransactionHandler &seedTransactionHandler,
                       HttpClientHandler &clientHandler,
                       ESB::Allocator &allocator)
    : _threads(0 >= threads ? 1 : threads),
      _connections(connections),
      _state(ES_HTTP_CLIENT_IS_DESTROYED),
      _allocator(allocator),
      _seedTransactionHandler(seedTransactionHandler),
      _clientHandler(clientHandler),
      _multiplexers(),
      _threadPool("HttpClientMultiplexerPool", _threads),
      _clientCounters(60, 1, _allocator) {}

HttpClient::~HttpClient() {}

ESB::Error HttpClient::pushAll(HttpClientCommand *command) {
  if (ES_HTTP_CLIENT_IS_STARTED != _state.get()) {
    return ESB_INVALID_STATE;
  }

  if (!command) {
    return ESB_NULL_POINTER;
  }

  if (command->cleanupHandler()) {
    // must return NULL else all multiplexers will try to destroy/free the same
    // command.
    return ESB_INVALID_ARGUMENT;
  }

  ESB::Error result = ESB_SUCCESS;

  for (ESB::ListIterator it = _multiplexers.frontIterator(); !it.isNull();
       it = it.next()) {
    HttpClientMultiplexer *multiplexer = (HttpClientMultiplexer *)it.value();
    ESB::Error error = multiplexer->push(command);

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot queue command on multiplexer");
      result = error;
    }
  }

  return result;
}

ESB::Error HttpClient::initialize() {
  assert(ES_HTTP_CLIENT_IS_DESTROYED == _state.get());
  _state.set(ES_HTTP_CLIENT_IS_INITIALIZED);
  return ESB_SUCCESS;
}

ESB::Error HttpClient::start() {
  assert(ES_HTTP_CLIENT_IS_INITIALIZED == _state.get());

  ESB::Error error = _threadPool.start();

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot start multiplexer threads");
    return error;
  }

  ESB::UInt32 maxSockets = ESB::SystemConfig::Instance().socketSoftMax();
  ESB_LOG_NOTICE("Maximum sockets %u", maxSockets);

  for (ESB::UInt32 i = 0; i < _threads; ++i) {
    ESB::UInt32 connections = _connections / _threads;
    if (i + 1 == _threads) {
      connections += _connections % _threads;
    }
    ESB::SocketMultiplexer *multiplexer = new (_allocator)
        HttpClientMultiplexer(connections, _seedTransactionHandler, maxSockets,
                              _clientHandler, _clientCounters);

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

  _state.set(ES_HTTP_CLIENT_IS_STARTED);
  ESB_LOG_NOTICE("Started");
  return ESB_SUCCESS;
}

ESB::Error HttpClient::stop() {
  assert(ES_HTTP_CLIENT_IS_STARTED == _state.get());
  _state.set(ES_HTTP_CLIENT_IS_STOPPED);

  ESB_LOG_NOTICE("Stopping");
  _threadPool.stop();
  ESB_LOG_NOTICE("Stopped");

  return ESB_SUCCESS;
}

void HttpClient::destroy() {
  assert(ES_HTTP_CLIENT_IS_STOPPED == _state.get());
  _state.set(ES_HTTP_CLIENT_IS_DESTROYED);

  for (ESB::ListIterator it = _multiplexers.frontIterator(); !it.isNull();
       it = it.next()) {
    HttpClientMultiplexer *multiplexer = (HttpClientMultiplexer *)it.value();
    multiplexer->~HttpClientMultiplexer();
    _allocator.deallocate(multiplexer);
  }

  _multiplexers.clear();
}

}  // namespace ES
