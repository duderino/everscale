#ifndef ES_HTTP_PROXY_H
#include <ESHttpProxy.h>
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

HttpProxy::HttpProxy(const char *namePrefix, ESB::UInt32 threads, ESB::UInt32 idleTimeoutMsec,
                     HttpProxyHandler &proxyHandler, ESB::Allocator &allocator)
    : HttpServer(namePrefix, threads, idleTimeoutMsec, proxyHandler, allocator),
      _proxyHandler(proxyHandler),
      _clientContextIndex(HttpConfig::Instance().tlsContextBuckets(), HttpConfig::Instance().tlsContextLocks(),
                          _allocator),
      _clientCounters(60, 1, _allocator) {}

HttpProxy::~HttpProxy() {}

ESB::Error HttpProxy::push(HttpClientCommand *command, int idx) {
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
  ESB::Error error = multiplexer->pushClientCommand(command);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot queue command on multiplexer");
    return error;
  }

  return ESB_SUCCESS;
}

ESB::SocketMultiplexer *HttpProxy::createMultiplexer() {
  return new (_allocator)
      HttpProxyMultiplexer(_name, ESB::SystemConfig::Instance().socketSoftMax(), _idleTimeoutMsec, _proxyHandler,
                           _proxyHandler, _clientCounters, _serverCounters, _clientContextIndex, _serverContextIndex);
}

}  // namespace ES
