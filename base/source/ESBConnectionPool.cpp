#ifndef ESB_CONNECTION_POOL_H
#include <ESBConnectionPool.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
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
      _activeConnections(_callbacks, numBuckets, numLocks, allocator) {}

ConnectionPool::~ConnectionPool() { clear(); }

void ConnectionPool::clear() {
  for (EmbeddedListElement *e = _deconstructedTCPConnections.removeFirst(); e;
       e = _deconstructedTCPConnections.removeFirst()) {
    _allocator.deallocate(e);
  }
  for (EmbeddedListElement *e = _deconstructedTLSConnections.removeFirst(); e;
       e = _deconstructedTLSConnections.removeFirst()) {
    _allocator.deallocate(e);
  }
  _activeConnections.clear();
  _hits.set(0);
  _misses.set(0);
}

Error ConnectionPool::acquire(const SocketAddress &peerAddress, ConnectedSocket **connection, bool *reused) {
  if (!connection || !reused) {
    return ESB_NULL_POINTER;
  }

  *connection = (ConnectedSocket *)_activeConnections.remove(&peerAddress);

  if (*connection) {
    assert((*connection)->peerAddress() == peerAddress);
    _hits.inc();
    *reused = true;
    return ESB_SUCCESS;
  }

  _misses.inc();
  *reused = false;

  switch (peerAddress.type()) {
    case SocketAddress::TLS:
      // TODO support TLS
    case SocketAddress::TCP: {
      EmbeddedListElement *memory = _deconstructedTCPConnections.removeLast();

      if (memory) {
        *connection = new (memory) ConnectedSocket(_namePrefix, _nameSuffix, peerAddress, false);
      } else {
        *connection = new (_allocator) ConnectedSocket(_namePrefix, _nameSuffix, peerAddress, false);
      }

      Error error = *connection ? (*connection)->connect() : ESB_OUT_OF_MEMORY;

      if (ESB_SUCCESS != error) {
        ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot connect to peer", (*connection)->name());
        (*connection)->~ConnectedSocket();
        _deconstructedTCPConnections.addLast(*connection);
        *connection = NULL;
        return error;
      }

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
    Error error = _activeConnections.insert(connection);
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
      _deconstructedTCPConnections.addLast(connection);
      break;
    case ESB::SocketAddress::TLS:
      _deconstructedTLSConnections.addLast(connection);
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
