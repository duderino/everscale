#ifndef ESB_CONNECTION_POOL_H
#include <ESBConnectionPool.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_CLIENT_TLS_SOCKET_H
#include <ESBClientTLSSocket.h>
#endif

#ifndef ESB_CLEAR_SOCKET_H
#include <ESBClearSocket.h>
#endif

namespace ESB {

ConnectionPool::ConnectionPool(const char *prefix, UInt32 numBuckets, UInt32 numLocks, Allocator &allocator)
    : _prefix(prefix),
      _allocator(allocator),
      _hits(0),
      _misses(0),
      _callbacks(allocator),
      _activeSockets(_callbacks, numBuckets, numLocks, allocator) {}

ConnectionPool::~ConnectionPool() { clear(); }

void ConnectionPool::clear() {
  for (EmbeddedListElement *e = _deconstructedClearSockets.removeFirst(); e;
       e = _deconstructedClearSockets.removeFirst()) {
    _allocator.deallocate(e);
  }
  for (EmbeddedListElement *e = _deconstructedTLSSockets.removeFirst(); e; e = _deconstructedTLSSockets.removeFirst()) {
    _allocator.deallocate(e);
  }
  _activeSockets.clear();
  _hits.set(0);
  _misses.set(0);
}

Error ConnectionPool::acquireClearSocket(const SocketAddress &peerAddress,
                                         ConnectedSocket **connection,
                                         bool *reused) {
  if (!connection || !reused) {
    return ESB_NULL_POINTER;
  }

  assert(SocketAddress::TCP == peerAddress.type());
  if (SocketAddress::TCP != peerAddress.type()) {
    return ESB_INVALID_ARGUMENT;
  }

  ClearSocket *socket = (ClearSocket *)_activeSockets.remove(&peerAddress);

  if (socket) {
    assert(0 == socket->peerAddress().compare(peerAddress));
    _hits.inc();
    *reused = true;
    *connection = socket;
    return ESB_SUCCESS;
  }

  _misses.inc();

  EmbeddedListElement *memory = _deconstructedTLSSockets.removeLast();

  if (memory) {
    socket = new (memory) ClearSocket(peerAddress, _prefix);
  } else {
    socket = new (_allocator) ClearSocket(peerAddress, _prefix);
  }

  if (!socket) {
    if (ESB_ERROR_LOGGABLE) {
      char presentationAddress[ESB_IPV6_PRESENTATION_SIZE];
      peerAddress.presentationAddress(presentationAddress, sizeof(presentationAddress));
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "cannot connect to [%s:%u]", presentationAddress, peerAddress.port());
    }
    return ESB_OUT_OF_MEMORY;
  }

  Error error = socket->connect();

  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot connect to peer", socket->name());
    socket->~ClearSocket();
    _deconstructedTLSSockets.addLast(socket);
    *connection = NULL;
    return error;
  }

  *reused = false;
  *connection = socket;
  return ESB_SUCCESS;
}

Error ConnectionPool::acquireTLSSocket(const HostAddress &peerAddress, ConnectedSocket **connection, bool *reused) {
  if (!connection || !reused) {
    return ESB_NULL_POINTER;
  }

  assert(SocketAddress::TLS == peerAddress.type());
  if (SocketAddress::TLS != peerAddress.type()) {
    return ESB_INVALID_ARGUMENT;
  }

  ClientTLSSocket *socket = (ClientTLSSocket *)_activeSockets.remove(&peerAddress);

    if (socket) {
      assert(0 == socket->peerAddress().compare(peerAddress));
      _hits.inc();
      *reused = true;
      *connection = socket;
      return ESB_SUCCESS;
    }

  _misses.inc();

  EmbeddedListElement *memory = _deconstructedTLSSockets.removeLast();

  if (memory) {
    socket = new (memory) ClientTLSSocket(peerAddress, _prefix);
  } else {
    socket = new (_allocator) ClientTLSSocket(peerAddress, _prefix);
  }

  if (!socket) {
    if (ESB_ERROR_LOGGABLE) {
      char presentationAddress[ESB_IPV6_PRESENTATION_SIZE];
      peerAddress.presentationAddress(presentationAddress, sizeof(presentationAddress));
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "cannot connect to [%s:%u]", presentationAddress, peerAddress.port());
    }
    return ESB_OUT_OF_MEMORY;
  }

  Error error = socket->connect();

  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot connect to peer", socket->name());
    socket->~ClientTLSSocket();
    _deconstructedTLSSockets.addLast(socket);
    *connection = NULL;
    return error;
  }

  *reused = false;
  *connection = socket;
  return ESB_SUCCESS;
}

void ConnectionPool::release(ConnectedSocket *connection) {
  if (!connection) {
    return;
  }

  if (connection->connected()) {
    Error error = _activeSockets.insert(connection);
    if (ESB_SUCCESS == error) {
      return;
    }
    ESB_LOG_WARNING_ERRNO(error, "[%s] Cannot return connection to connection pool", connection->name());
    connection->close();
  }

  assert(!connection->connected());
  SocketAddress::TransportType type = connection->peerAddress().type();
  connection->~ConnectedSocket();

  switch (type) {
    case ESB::SocketAddress::TCP:
      _deconstructedClearSockets.addLast(connection);
      break;
    case ESB::SocketAddress::TLS:
      _deconstructedTLSSockets.addLast(connection);
      break;
    default:
      _allocator.deallocate(connection);
  }
}

ConnectionPool::SocketAddressCallbacks::SocketAddressCallbacks(ESB::Allocator &allocator) : _allocator(allocator) {}

int ConnectionPool::SocketAddressCallbacks::compare(const void *f, const void *s) const {
  return ((ESB::SocketAddress *)f)->compare(*((ESB::SocketAddress *) s));
}

ESB::UInt32 ConnectionPool::SocketAddressCallbacks::hash(const void *key) const {
  return ((ESB::SocketAddress *)key)->hash();
}

void ConnectionPool::SocketAddressCallbacks::cleanup(ESB::EmbeddedMapElement *element) {
  ConnectedSocket *connection = (ConnectedSocket *)element;
  connection->close();
  connection->~ConnectedSocket();
  _allocator.deallocate(connection);
}

}  // namespace ESB
