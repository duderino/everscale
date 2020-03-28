#ifndef ES_HTTP_CLIENT_SOCKET_FACTORY_H
#include <ESHttpClientSocketFactory.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_FIXED_ALLOCATOR_H
#include <ESBFixedAllocator.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_NULL_LOCK_H
#include <ESBNullLock.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

namespace ES {

class AddressComparator : public ESB::Comparator {
 public:
  /** Default constructor.
   */
  AddressComparator();

  /** Default destructor.
   */
  virtual ~AddressComparator();

  /** Compare two locations.
   *
   *  @param first The first location to compare.
   *  @param second The second location to compare.
   *  @return 0 if both locations are equal, a negative number if the first
   *      location is less than the second, or a positive number if the first
   *      location is greater than the second.
   */
  virtual int compare(const void *first, const void *second) const;
};

AddressComparator::AddressComparator() {}

AddressComparator::~AddressComparator() {}

int AddressComparator::compare(const void *first, const void *second) const {
  ESB::SocketAddress *address1 = (ESB::SocketAddress *)first;
  ESB::SocketAddress *address2 = (ESB::SocketAddress *)second;

  assert(address1);
  assert(address2);

  return memcmp(address1->primitiveAddress(), address2->primitiveAddress(),
                sizeof(*address1->primitiveAddress()));
}

static AddressComparator AddressComparator;

HttpClientSocketFactory::HttpClientSocketFactory(
    HttpClientCounters *clientCounters)
    : _clientCounters(clientCounters),
      _unprotectedAllocator(ESB::SystemConfig::Instance().pageSize(),
                            ESB::SystemConfig::Instance().cacheLineSize()),
      _allocator(_unprotectedAllocator),
      _map(AddressComparator),
      _embeddedList(),
      _mutex(),
      _cleanupHandler(this) {}

ESB::Error HttpClientSocketFactory::initialize() { return ESB_SUCCESS; }

void HttpClientSocketFactory::destroy() {
  HttpClientSocket *socket = (HttpClientSocket *)_embeddedList.removeFirst();

  while (socket) {
    socket->~HttpClientSocket();
    socket = (HttpClientSocket *)_embeddedList.removeFirst();
  }

  ESB::SocketAddress *address = 0;

  for (ESB::MapIterator iterator = _map.minimumIterator();
       false == iterator.isNull(); iterator = iterator.next()) {
    address = (ESB::SocketAddress *)iterator.key();

    assert(address);

    address->~SocketAddress();

    socket = (HttpClientSocket *)iterator.value();

    assert(socket);

    socket->~HttpClientSocket();
  }
}

HttpClientSocketFactory::~HttpClientSocketFactory() {}

HttpClientSocket *HttpClientSocketFactory::create(
    HttpConnectionPool *pool, HttpClientTransaction *transaction) {
  HttpClientSocket *socket = 0;

  {
    ESB::WriteScopeLock scopeLock(_mutex);
    ESB::EmbeddedList *list =
        (ESB::EmbeddedList *)_map.find(&transaction->peerAddress());

    if (list) {
      socket = (HttpClientSocket *)list->removeLast();

      if (socket) {
        if (ESB_DEBUG_LOGGABLE) {
          char buffer[ESB_IPV6_PRESENTATION_SIZE];
          transaction->peerAddress().presentationAddress(buffer,
                                                         sizeof(buffer));
          ESB_LOG_DEBUG("Reusing connection for '%s:%d'", buffer,
                        transaction->peerAddress().port());
        }

        assert(socket->isConnected());
        socket->reset(true, pool, transaction);
        return socket;
      }
    }

    if (ESB_DEBUG_LOGGABLE) {
      char buffer[ESB_IPV6_PRESENTATION_SIZE];
      transaction->peerAddress().presentationAddress(buffer, sizeof(buffer));
      ESB_LOG_DEBUG("Creating new connection for '%s:%d'", buffer,
                    transaction->peerAddress().port());
    }

    // Try to reuse the memory of a dead socket
    socket = (HttpClientSocket *)_embeddedList.removeLast();

    if (socket) {
      assert(false == socket->isConnected());
      socket->reset(false, pool, transaction);

      return socket;
    }
  }

  // Failing all else, allocate a new socket
  socket = new (_allocator)
      HttpClientSocket(pool, transaction, _clientCounters, &_cleanupHandler);

  if (!socket && ESB_CRITICAL_LOGGABLE) {
    char buffer[ESB_IPV6_PRESENTATION_SIZE];
    transaction->peerAddress().presentationAddress(buffer, sizeof(buffer));
    ESB_LOG_CRITICAL("Cannot allocate new connection for '%s:%d'", buffer,
                     transaction->peerAddress().port());
  }

  return socket;
}

void HttpClientSocketFactory::release(HttpClientSocket *socket) {
  if (!socket) {
    return;
  }

  ESB::WriteScopeLock scopeLock(_mutex);

  if (false == socket->isConnected()) {
    if (ESB_DEBUG_LOGGABLE) {
      char buffer[ESB_IPV6_PRESENTATION_SIZE];
      socket->peerAddress()->presentationAddress(buffer, sizeof(buffer));
      ESB_LOG_DEBUG("Not returning connection to '%s:%d' to pool", buffer,
                    socket->peerAddress()->port());
    }

    socket->close();  // idempotent, just to be safe
    _embeddedList.addLast(socket);
    return;
  }

  ESB::EmbeddedList *list =
      (ESB::EmbeddedList *)_map.find(socket->peerAddress());

  if (list) {
    if (ESB_DEBUG_LOGGABLE) {
      char buffer[ESB_IPV6_PRESENTATION_SIZE];
      socket->peerAddress()->presentationAddress(buffer, sizeof(buffer));
      ESB_LOG_DEBUG("Returning connection to '%s:%d' to pool", buffer,
                    socket->peerAddress()->port());
    }

    list->addLast(socket);
    return;
  }

  ESB::SocketAddress *address =
      new (_allocator) ESB::SocketAddress(*socket->peerAddress());

  if (!address) {
    if (ESB_CRITICAL_LOGGABLE) {
      char buffer[ESB_IPV6_PRESENTATION_SIZE];
      socket->peerAddress()->presentationAddress(buffer, sizeof(buffer));
      ESB_LOG_CRITICAL("Cannot return connection to '%s:%d' to pool: bad alloc",
                       buffer, socket->peerAddress()->port());
    }

    socket->close();
    _embeddedList.addLast(socket);
    return;
  }

  list = new (_allocator) ESB::EmbeddedList();

  if (0 == list) {
    if (ESB_WARNING_LOGGABLE) {
      char buffer[ESB_IPV6_PRESENTATION_SIZE];
      socket->peerAddress()->presentationAddress(buffer, sizeof(buffer));
      ESB_LOG_WARNING("Cannot return connection to '%s:%d' to pool: bad alloc",
                      buffer, socket->peerAddress()->port());
    }

    socket->close();
    _embeddedList.addLast(socket);
    return;
  }

  list->addLast(socket);

  ESB::Error error = _map.insert(address, list);

  if (ESB_SUCCESS != error) {
    if (ESB_WARNING_LOGGABLE) {
      char dottedIP[ESB_IPV6_PRESENTATION_SIZE];
      socket->peerAddress()->presentationAddress(dottedIP, sizeof(dottedIP));
      ESB_LOG_WARNING_ERRNO(error,
                            "Cannot return connection to '%s:%d' to pool",
                            dottedIP, socket->peerAddress()->port());
    }

    socket->close();
    _embeddedList.addLast(socket);
  }
}

HttpClientSocketFactory::CleanupHandler::CleanupHandler(
    HttpClientSocketFactory *factory)
    : ESB::CleanupHandler(), _factory(factory) {}

HttpClientSocketFactory::CleanupHandler::~CleanupHandler() {}

void HttpClientSocketFactory::CleanupHandler::destroy(ESB::Object *object) {
  _factory->release((HttpClientSocket *)object);
}

}  // namespace ES
