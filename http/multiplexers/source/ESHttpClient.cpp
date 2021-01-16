#ifndef ES_HTTP_CLIENT_H
#include <ESHttpClient.h>
#endif

#ifndef ES_HTTP_PROXY_MULTIPLEXER_H
#include <ESHttpProxyMultiplexer.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

namespace ES {

HttpClient::HttpClient(const char *namePrefix, ESB::UInt32 threads, HttpClientHandler &clientHandler,
                       ESB::Allocator &allocator)
    : _threads(0 >= threads ? 1 : threads),
      _state(ES_HTTP_CLIENT_IS_DESTROYED),
      _allocator(allocator),
      _clientHandler(clientHandler),
      _multiplexers(),
      _threadPool(namePrefix, _threads),
      _rand(),
      _clientCounters(5*60, 1, _allocator) {
  strncpy(_name, namePrefix, sizeof(_name));
  _name[sizeof(_name) - 1] = 0;
}

HttpClient::~HttpClient() {}

ESB::Error HttpClient::push(HttpClientCommand *command, int idx) {
  if (ES_HTTP_CLIENT_IS_STARTED != _state.get()) {
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
  ESB::Error error = multiplexer->pushClientCommand(command);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "[%s] cannot queue command on multiplexer", _name);
    return error;
  }

  return ESB_SUCCESS;
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
    ESB_LOG_CRITICAL_ERRNO(error, "[%s] cannot start multiplexer threads", _name);
    return error;
  }

  ESB::UInt32 maxSockets = ESB::SystemConfig::Instance().socketSoftMax();
  ESB_LOG_DEBUG("[%s] maximum sockets %u", _name, maxSockets);

  for (ESB::UInt32 i = 0; i < _threads; ++i) {
    ESB::SocketMultiplexer *multiplexer =
        new (_allocator) HttpProxyMultiplexer(_name, maxSockets, _clientHandler, _clientCounters);

    if (!multiplexer) {
      ESB_LOG_CRITICAL_ERRNO(ESB_OUT_OF_MEMORY, "Cannot initialize multiplexer");
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

  _state.set(ES_HTTP_CLIENT_IS_STARTED);
  ESB_LOG_DEBUG("[%s] started", _name);
  return ESB_SUCCESS;
}

ESB::Error HttpClient::stop() {
  assert(ES_HTTP_CLIENT_IS_STARTED == _state.get());
  _state.set(ES_HTTP_CLIENT_IS_STOPPED);

  ESB_LOG_DEBUG("[%s] stopping", _name);
  _threadPool.stop();
  ESB_LOG_DEBUG("[%s] stopped", _name);

  return ESB_SUCCESS;
}

void HttpClient::destroy() {
  assert(ES_HTTP_CLIENT_IS_STOPPED == _state.get());
  _state.set(ES_HTTP_CLIENT_IS_DESTROYED);

  for (ESB::ListIterator it = _multiplexers.frontIterator(); !it.isNull(); it = it.next()) {
    HttpProxyMultiplexer *multiplexer = (HttpProxyMultiplexer *)it.value();
    multiplexer->~HttpProxyMultiplexer();
    _allocator.deallocate(multiplexer);
  }

  _multiplexers.clear();
}

}  // namespace ES
