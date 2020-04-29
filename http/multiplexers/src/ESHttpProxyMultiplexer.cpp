#ifndef ES_HTTP_PROXY_MULTIPLEXER_H
#include <ESHttpProxyMultiplexer.h>
#endif

#ifndef ES_HTTP_LISTENING_SOCKET_H
#include <ESHttpListeningSocket.h>
#endif

namespace ES {

HttpProxyMultiplexer::HttpProxyMultiplexer(ESB::UInt32 maxSockets,
                                           HttpProxyHandler &proxyHandler,
                                           HttpClientCounters &clientCounters,
                                           HttpServerCounters &serverCounters)
    : HttpServerMultiplexer(maxSockets, proxyHandler, serverCounters),
      _clientSocketFactory(*this, proxyHandler, clientCounters,
                           _factoryAllocator),
      _clientTransactionFactory(_factoryAllocator),
      _clientStack(_multiplexer, _clientSocketFactory,
                   _clientTransactionFactory, _ioBufferPool),
      _clientCommandSocket(_clientStack) {
  _clientSocketFactory.setStack(_clientStack);
}

HttpProxyMultiplexer::~HttpProxyMultiplexer() {}

bool HttpProxyMultiplexer::run(ESB::SharedInt *isRunning) {
  ESB::Error error = _multiplexer.addMultiplexedSocket(&_clientCommandSocket);

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot add command socket to multiplexer");
    return false;
  }

  error = _multiplexer.addMultiplexedSocket(&_serverCommandSocket);

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot add command socket to multiplexer");
    return false;
  }

  return _multiplexer.run(isRunning);
}

const char *HttpProxyMultiplexer::name() const { return _multiplexer.name(); }

ESB::CleanupHandler *HttpProxyMultiplexer::cleanupHandler() { return NULL; }

}  // namespace ES
