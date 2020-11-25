#ifndef ESB_CONNECTION_POOL_H
#include <ESBConnectionPool.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_BORING_SSL_SOCKET_H
#include <ESBBoringSSLSocket.h>
#endif

namespace ESB {

ConnectionPool::ConnectionPool(const char *namePrefix, const char *nameSuffix, UInt32 numBuckets, UInt32 numLocks,
                               Allocator &allocator)
    : _namePrefix(namePrefix),
      _nameSuffix(nameSuffix),
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

Error ConnectionPool::acquire(const SocketAddress &peerAddress, ConnectedSocket **connection, bool *reused) {
  if (!connection || !reused) {
    return ESB_NULL_POINTER;
  }

  {
    ConnectedSocket *socket = (ConnectedSocket *)_activeSockets.remove(&peerAddress);

    if (socket) {
      assert(socket->peerAddress() == peerAddress);
      _hits.inc();
      *reused = true;
      *connection = socket;
      return ESB_SUCCESS;
    }
  }

  _misses.inc();

  switch (peerAddress.type()) {
    case SocketAddress::TLS: {
      EmbeddedListElement *memory = _deconstructedTLSSockets.removeLast();
      BoringSSLSocket *socket = NULL;

      if (memory) {
        socket = new (memory) BoringSSLSocket(_namePrefix, _nameSuffix, peerAddress, false);
      } else {
        socket = new (_allocator) BoringSSLSocket(_namePrefix, _nameSuffix, peerAddress, false);
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
        socket->~BoringSSLSocket();
        _deconstructedTLSSockets.addLast(socket);
        *connection = NULL;
        return error;
      }

      *reused = false;
      *connection = socket;
      return ESB_SUCCESS;
    }
    case SocketAddress::TCP: {
      EmbeddedListElement *memory = _deconstructedClearSockets.removeLast();
      ConnectedSocket *socket = NULL;

      if (memory) {
        socket = new (memory) ConnectedSocket(_namePrefix, _nameSuffix, peerAddress, false);
      } else {
        socket = new (_allocator) ConnectedSocket(_namePrefix, _nameSuffix, peerAddress, false);
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
        socket->~ConnectedSocket();
        _deconstructedClearSockets.addLast(socket);
        *connection = NULL;
        return error;
      }

      *reused = false;
      *connection = socket;
      return ESB_SUCCESS;
    }
    default:
      return ESB_UNSUPPORTED_TRANSPORT;
  }
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
  connection->~ConnectedSocket();

  switch (connection->peerAddress().type()) {
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
  ESB::SocketAddress *first = (ESB::SocketAddress *)f;
  ESB::SocketAddress *second = (ESB::SocketAddress *)s;

  if (first->type() != second->type()) {
    return first->type() - second->type();
  }

  // TODO not IPv6 safe
  return memcmp(first->primitiveAddress(), second->primitiveAddress(), sizeof(ESB::SocketAddress::Address));
}

ESB::UInt32 ConnectionPool::SocketAddressCallbacks::hash(const void *key) const {
  ESB::SocketAddress *addr = (ESB::SocketAddress *)key;

  // TODO not IPv6 safe, for IPv6 take the low order bits of the address
  ESB::UInt32 hash = addr->primitiveAddress()->sin_addr.s_addr;
  hash |= addr->primitiveAddress()->sin_port << 16;
  hash |= addr->primitiveAddress()->sin_family << 30;
  hash |= addr->type() << 31;

  return hash;
}

void ConnectionPool::SocketAddressCallbacks::cleanup(ESB::EmbeddedMapElement *element) {
  ConnectedSocket *connection = (ConnectedSocket *)element;
  connection->close();
  connection->~ConnectedSocket();
  _allocator.deallocate(connection);
}

}  // namespace ESB
