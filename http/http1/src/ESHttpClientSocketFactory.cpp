#ifndef ES_HTTP_CLIENT_SOCKET_FACTORY_H
#include <ESHttpClientSocketFactory.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ES_HTTP_CONFIG_H
#include <ESHttpConfig.h>
#endif

namespace ES {

HttpClientSocketFactory::SocketAddressCallbacks::SocketAddressCallbacks(
    ESB::Allocator &allocator)
    : _allocator(allocator) {}

int HttpClientSocketFactory::SocketAddressCallbacks::compare(
    const void *f, const void *s) const {
  ESB::SocketAddress::Address *first = ((ESB::SocketAddress *)f)->primitiveAddress();
  ESB::SocketAddress::Address *second = ((ESB::SocketAddress *)s)->primitiveAddress();

  // TODO not IPv6 safe

  if (first->sin_port > second->sin_port) {
    return 1;
  } else if (second->sin_port > first->sin_port) {
    return -1;
  }

  if (first->sin_addr.s_addr > second->sin_addr.s_addr) {
    return 1;
  } else if (second->sin_addr.s_addr > first->sin_addr.s_addr) {
    return -1;
  }

  if (first->sin_family > second->sin_family) {
    return 1;
  } else if (second->sin_family > first->sin_family) {
    return -1;
  }

  return 0;
}

ESB::UInt32 HttpClientSocketFactory::SocketAddressCallbacks::hash(
    const void *key) const {
  ESB::SocketAddress *addr = (ESB::SocketAddress *)key;

  // TODO not IPv6 safe
  ESB::UInt32 hash = addr->primitiveAddress()->sin_addr.s_addr;
  hash |= addr->primitiveAddress()->sin_family;
  hash |= addr->primitiveAddress()->sin_port;
  return hash;
}

void HttpClientSocketFactory::SocketAddressCallbacks::cleanup(
    ESB::EmbeddedMapElement *element) {
  element->~EmbeddedMapElement();
  _allocator.deallocate(element);
}

HttpClientSocketFactory::HttpClientSocketFactory(
    ESB::SocketMultiplexer &multiplexer, HttpClientHandler &handler,
    HttpClientCounters &counters, ESB::Allocator &allocator)
    : _multiplexer(multiplexer),
      _handler(handler),
      _counters(counters),
      _allocator(allocator),
      _callbacks(_allocator),
      _map(_callbacks, HttpConfig::Instance().connectionPoolBuckets(), 0),
      _cleanupHandler(*this),
      _dnsClient() {}

HttpClientSocketFactory::~HttpClientSocketFactory() {
  while (true) {
    HttpClientSocket *socket = (HttpClientSocket *)_sockets.removeFirst();
    if (!socket) {
      break;
    }
    socket->~HttpClientSocket();
    _allocator.deallocate(socket);
  }
}

HttpClientSocket *HttpClientSocketFactory::create(
    HttpClientTransaction *transaction) {
  if (!transaction || !_stack) {
    return NULL;
  }

  HttpClientSocket *socket =
      (HttpClientSocket *)_map.remove(&transaction->peerAddress());

  if (socket) {
    assert(socket->isConnected());
    socket->reset(true, transaction);
    ESB_LOG_DEBUG("[%s] reusing connection", socket->logAddress());
    return socket;
  }

  if (ESB_DEBUG_LOGGABLE) {
    char buffer[ESB_IPV6_PRESENTATION_SIZE];
    transaction->peerAddress().presentationAddress(buffer, sizeof(buffer));
    ESB_LOG_DEBUG("[%s:%u] creating new connection", buffer,
                  transaction->peerAddress().port());
  }

  // Try to reuse the memory of a dead socket
  socket = (HttpClientSocket *)_sockets.removeLast();

  if (socket) {
    assert(!socket->isConnected());
    socket->reset(false, transaction);
    return socket;
  }

  socket = new (_allocator)
      HttpClientSocket(_handler, *_stack, transaction->peerAddress(), _counters,
                       _cleanupHandler);

  if (!socket) {
    char dottedIP[ESB_IPV6_PRESENTATION_SIZE];
    transaction->peerAddress().presentationAddress(dottedIP, sizeof(dottedIP));
    ESB_LOG_CRITICAL_ERRNO(ESB_OUT_OF_MEMORY,
                           "[%s:%d] cannot create new connection", dottedIP,
                           transaction->peerAddress().port());
    return socket;
  }

  socket->reset(false, transaction);

  return socket;
}

void HttpClientSocketFactory::release(HttpClientSocket *socket) {
  if (!socket) {
    return;
  }

  if (!socket->isConnected()) {
    ESB_LOG_DEBUG("[%s] Not returning connection to pool",
                  socket->logAddress());
    socket->close();  // idempotent, just to be safe
    _sockets.addLast(socket);
    return;
  }

  ESB::Error error = _map.insert(socket);

  if (ESB_SUCCESS != error) {
    ESB_LOG_WARNING_ERRNO(error, "[%s] Cannot return connection to pool",
                          socket->logAddress());
    socket->close();
    _sockets.addLast(socket);
  }
}

ESB::Error HttpClientSocketFactory::executeClientTransaction(
    HttpClientTransaction *transaction) {
  // don't uncomment the end handler callbacks until after the processing goes
  // asynch
  assert(transaction);
  if (!transaction) {
    return ESB_NULL_POINTER;
  }

  transaction->setStartTime();

  // TODO Make resolver async
  unsigned char hostname[1024];
  hostname[0] = 0;
  ESB::UInt16 port = 0;
  bool isSecure = false;

  ESB::Error error = transaction->request().parsePeerAddress(
      hostname, sizeof(hostname), &port, &isSecure);

  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG("Cannot extract hostname from request");
    return error;
  }

  error =
      _dnsClient.resolve(transaction->peerAddress(), hostname, port, isSecure);

  if (ESB_SUCCESS != error) {
    _counters.getFailures()->record(transaction->startTime(), ESB::Date::Now());
    // transaction->getHandler()->end(transaction,
    //                               HttpClientHandler::ES_HTTP_CLIENT_HANDLER_RESOLVE);
    return error;
  }

  HttpClientSocket *socket = create(transaction);

  if (!socket) {
    _counters.getFailures()->record(transaction->startTime(), ESB::Date::Now());
    // transaction->getHandler()->end(transaction,
    //                               HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
    ESB_LOG_CRITICAL("Cannot allocate new client socket");
    return 0;
  }

  if (!socket->isConnected()) {
    error = socket->connect();

    if (ESB_SUCCESS != error) {
      _counters.getFailures()->record(transaction->startTime(),
                                      ESB::Date::Now());
      ESB_LOG_WARNING_ERRNO(error, "[%s] Cannot connect", socket->logAddress());
      // transaction->getHandler()->end(transaction,
      //                               HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
      socket->close();
      release(socket);
      return error;
    }
  }

  error = _multiplexer.addMultiplexedSocket(socket);

  if (ESB_SUCCESS != error) {
    _counters.getFailures()->record(transaction->startTime(), ESB::Date::Now());
    ESB_LOG_CRITICAL_ERRNO(error,
                           "[%s] Cannot add client socket to multiplexer",
                           socket->logAddress());
    socket->close();
    // transaction->getHandler()->end(transaction,
    //                               HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
    release(socket);
    return error;
  }

  return ESB_SUCCESS;
}

HttpClientSocketFactory::CleanupHandler::CleanupHandler(
    HttpClientSocketFactory &factory)
    : ESB::CleanupHandler(), _factory(factory) {}

HttpClientSocketFactory::CleanupHandler::~CleanupHandler() {}

void HttpClientSocketFactory::CleanupHandler::destroy(ESB::Object *object) {
  _factory.release((HttpClientSocket *)object);
}

}  // namespace ES
