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

ConnectionPool::ConnectionPool(const char *prefix, UInt32 numBuckets, UInt32 numLocks,
                               ClientTLSContextIndex &contextIndex, Allocator &allocator)
    : _prefix(prefix),
      _allocator(allocator),
      _contextIndex(contextIndex),
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

Error ConnectionPool::acquireClearSocket(const SocketAddress &peerAddress, ConnectedSocket **connection, bool *reused) {
  if (!connection || !reused) {
    return ESB_NULL_POINTER;
  }

  assert(SocketAddress::TCP == peerAddress.type());
  if (SocketAddress::TCP != peerAddress.type()) {
    return ESB_INVALID_ARGUMENT;
  }

  ClearSocket *socket = NULL;

  {
    SocketKey key(peerAddress, SocketKey::CLEAR_KEY, NULL);
    socket = (ClearSocket *)_activeSockets.remove(&key);
  }

  if (socket) {
    assert(0 == socket->peerAddress().compare(peerAddress));
    _hits.inc();
    *reused = true;
    *connection = socket;
    return ESB_SUCCESS;
  }

  _misses.inc();

  {
    ClearSocket *memory = (ClearSocket *)_deconstructedClearSockets.removeLast();
    if (memory) {
      socket = new (memory) ClearSocket(peerAddress, _prefix);
    } else {
      socket = new (_allocator) ClearSocket(peerAddress, _prefix);
    }
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
    _deconstructedClearSockets.addLast(socket);
    *connection = NULL;
    return error;
  }

  *reused = false;
  *connection = socket;
  return ESB_SUCCESS;
}

class TLSKey {
 public:
  TLSKey(const char *fqdn, TLSContextPointer &context) : _fqdn(fqdn), _context(context) {}

  const char *_fqdn;
  TLSContextPointer &_context;
};

Error ConnectionPool::acquireTLSSocket(const char *fqdn, const SocketAddress &peerAddress, ConnectedSocket **connection,
                                       bool *reused) {
  if (!connection || !reused) {
    return ESB_NULL_POINTER;
  }

  assert(SocketAddress::TLS == peerAddress.type());
  if (SocketAddress::TLS != peerAddress.type()) {
    return ESB_INVALID_ARGUMENT;
  }

  TLSContextPointer context;
  ESB::Error error = _contextIndex.matchContext(fqdn, context);
  switch (error) {
    case ESB_SUCCESS:
      break;
    case ESB_CANNOT_FIND:
      context = _contextIndex.defaultContext();
      if (context.isNull()) {
        ESB_LOG_WARNING("cannot find TLS client context for %s", fqdn);
        return ESB_CANNOT_FIND;
      }
      break;
    default:
      ESB_LOG_WARNING_ERRNO(error, "cannot find TLS client context for %s", fqdn);
      return error;
  }

  ClientTLSSocket *socket = NULL;

  {
    TLSKey tlsKey(fqdn, context);
    SocketKey key(peerAddress, SocketKey::TLS_KEY, &tlsKey);
    socket = (ClientTLSSocket *)_activeSockets.remove(&key);
  }

  if (socket) {
    assert(0 == socket->peerAddress().compare(peerAddress));
    _hits.inc();
    *reused = true;
    *connection = socket;
    return ESB_SUCCESS;
  }

  _misses.inc();

  {
    ClientTLSSocket *memory = (ClientTLSSocket *)_deconstructedTLSSockets.removeLast();
    if (memory) {
      socket = new (memory) ClientTLSSocket(fqdn, peerAddress, _prefix, context);
    } else {
      socket = new (_allocator) ClientTLSSocket(fqdn, peerAddress, _prefix, context);
    }
  }

  if (!socket) {
    if (ESB_ERROR_LOGGABLE) {
      char presentationAddress[ESB_IPV6_PRESENTATION_SIZE];
      peerAddress.presentationAddress(presentationAddress, sizeof(presentationAddress));
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "cannot connect to [%s:%u]", presentationAddress, peerAddress.port());
    }
    return ESB_OUT_OF_MEMORY;
  }

  error = socket->connect();

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
    if (SocketAddress::TLS == connection->peerAddress().type()) {
      // do not reuse TLS sockets if they haven't completed the handshake.
      ClientTLSSocket *socket = (ClientTLSSocket *)connection;
      X509Certificate *serverCertificate = NULL;
      Error error = socket->peerCertificate(&serverCertificate);
      if (ESB_SUCCESS != error) {
        ESB_LOG_DEBUG("[%s] discarding connection without peer cert from pool", connection->name());
        connection->~ConnectedSocket();
        _deconstructedTLSSockets.addLast(connection);
        return;
      }
    }

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
  SocketKey *key = (SocketKey *)f;
  SocketKey *object = (SocketKey *)s;

  assert(SocketKey::CLEAR_KEY == key->_tag || SocketKey::TLS_KEY == key->_tag);
  assert(SocketKey::CLEAR_OBJECT == object->_tag || SocketKey::TLS_OBJECT == object->_tag);

  if (SocketKey::CLEAR_KEY == key->_tag) {
    if (SocketKey::CLEAR_OBJECT == object->_tag) {
      // clear vs. clear
      return key->_peerAddress.compare(object->_peerAddress);
    } else {
      // clear vs. tls
      return -1;
    }
  }

  if (SocketKey::CLEAR_OBJECT == object->_tag) {
    // tls vs. clear
    return 1;
  }

  // tls vs. tls

  Word result = key->_peerAddress.compare(object->_peerAddress);
  if (0 != result) {
    return result;
  }

  TLSKey *tlsKey = (TLSKey *)key->_context;
  ClientTLSSocket *socket = (ClientTLSSocket *)object->_context;

  // client SSL_CTX must match (identity/same object)
  result = (Word)tlsKey->_context->rawContext() - (Word)socket->context()->rawContext();
  if (0 != result) {
    return result;
  }

  // hostname must be compatible with server cert

  X509Certificate *serverCertificate = NULL;
  Error error = socket->peerCertificate(&serverCertificate);
  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "[%s] Pooled connection has no server certificate", socket->name());
    return -1;
  }

  if (0 == serverCertificate->numSubjectAltNames()) {
    char cn[ESB_MAX_HOSTNAME + 1];
    error = serverCertificate->commonName(cn, sizeof(cn));
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "[%s] Pooled connection has no CN", socket->name());
      return -1;
    }
    return strcmp(cn, tlsKey->_fqdn);
  }

  char san[ESB_MAX_HOSTNAME + 1];
  UInt32 position = 0U;
  while (true) {
    error = serverCertificate->subjectAltName(san, sizeof(san), &position);
    switch (error) {
      case ESB_CANNOT_FIND:
        return -1;
      case ESB_SUCCESS: {
        int matchValue = StringWildcardMatch(san, tlsKey->_fqdn);
        if (0 <= matchValue) {
          return 0;
        }
        break;
      }
      default:
        ESB_LOG_ERROR_ERRNO(error, "[%s] cannot extract SAN from pooled connection", socket->name());
        return -1;
    }
  }
}

UInt64 ConnectionPool::SocketAddressCallbacks::hash(const void *k) const {
  SocketKey *key = (SocketKey *)k;
  UInt64 code = key->_peerAddress.hash();

  switch (key->_tag) {
    case SocketKey::CLEAR_KEY:
    case SocketKey::CLEAR_OBJECT:
      return code;
    case SocketKey::TLS_KEY: {
      TLSKey *tlsKey = (TLSKey *)key->_context;
      code *= 31 + (Word)tlsKey->_context->rawContext();
      return code;
    }
    case SocketKey::TLS_OBJECT: {
      ClientTLSSocket *socket = (ClientTLSSocket *)key->_context;
      code *= 31 + (Word)socket->context()->rawContext();
      return code;
    }
    default:
      ESB_LOG_ERROR("Unknown connection pool hash key tag: %d", key->_tag);
      assert(!"Unknown connection pool hash tag");
      return 0;
  }
}

void ConnectionPool::SocketAddressCallbacks::cleanup(ESB::EmbeddedMapElement *element) {
  ConnectedSocket *connection = (ConnectedSocket *)element;
  connection->close();
  connection->~ConnectedSocket();
  _allocator.deallocate(connection);
}

}  // namespace ESB
