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
  ESB::SocketAddress *address1 = (ESB::SocketAddress *) first;
  ESB::SocketAddress *address2 = (ESB::SocketAddress *) second;

  assert(address1);
  assert(address2);

  return memcmp(address1->getAddress(), address2->getAddress(),
                sizeof(*address1->getAddress()));
}

static AddressComparator AddressComparator;

HttpClientSocketFactory::HttpClientSocketFactory(HttpClientCounters *clientCounters)
    : _clientCounters(clientCounters),
      _unprotectedAllocator(ESB_WORD_ALIGN(sizeof(HttpClientSocket)) * 100,
                            ESB::SystemAllocator::GetInstance()),
      _allocator(&_unprotectedAllocator),
      _map(true, &AddressComparator,
           &_allocator,  // Will result in a leak if erase(), clear(), or
          // insert() with a uniqueness violation
           ESB::NullLock::Instance()),
      _embeddedList(),
      _mutex(),
      _cleanupHandler(this) {}

ESB::Error HttpClientSocketFactory::initialize() { return ESB_SUCCESS; }

void HttpClientSocketFactory::destroy() {
  HttpClientSocket *socket = (HttpClientSocket *) _embeddedList.removeFirst();

  while (socket) {
    socket->~HttpClientSocket();
    socket = (HttpClientSocket *) _embeddedList.removeFirst();
  }

  ESB::SocketAddress *address = 0;

  for (ESB::MapIterator iterator = _map.getMinimumIterator();
       false == iterator.isNull(); iterator = iterator.getNext()) {
    address = (ESB::SocketAddress *) iterator.getKey();

    assert(address);

    address->~SocketAddress();

    socket = (HttpClientSocket *) iterator.getValue();

    assert(socket);

    socket->~HttpClientSocket();
  }

  _allocator.destroy();
}

HttpClientSocketFactory::~HttpClientSocketFactory() {}

HttpClientSocket *HttpClientSocketFactory::create(
    HttpConnectionPool *pool, HttpClientTransaction *transaction) {
  HttpClientSocket *socket = 0;

  {
    ESB::WriteScopeLock scopeLock(_mutex);
    ESB::EmbeddedList *list =
        (ESB::EmbeddedList *) _map.find(transaction->getPeerAddress());

    if (list) {
      socket = (HttpClientSocket *) list->removeLast();

      if (socket) {
        if (ESB_DEBUG_LOGGABLE) {
          char buffer[ESB_IPV6_PRESENTATION_SIZE];
          transaction->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));
          ESB_LOG_DEBUG("Reusing connection for '%s:%d'", buffer,
                        transaction->getPeerAddress()->getPort());
        }

        assert(socket->isConnected());
        socket->reset(true, pool, transaction);
        return socket;
      }
    }

    if (ESB_DEBUG_LOGGABLE) {
      char buffer[ESB_IPV6_PRESENTATION_SIZE];
      transaction->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));
      ESB_LOG_DEBUG("Creating new connection for '%s:%d'", buffer,
                    transaction->getPeerAddress()->getPort());
    }

    // Try to reuse the memory of a dead socket
    socket = (HttpClientSocket *) _embeddedList.removeLast();

    if (socket) {
      assert(false == socket->isConnected());
      socket->reset(false, pool, transaction);

      return socket;
    }
  }

  // Failing all else, allocate a new socket
  socket = new(&_allocator) HttpClientSocket(
      pool, transaction, _clientCounters, &_cleanupHandler);

  if (!socket && ESB_CRITICAL_LOGGABLE) {
    char buffer[ESB_IPV6_PRESENTATION_SIZE];
    transaction->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));
    ESB_LOG_CRITICAL("Cannot allocate new connection for '%s:%d'", buffer,
                     transaction->getPeerAddress()->getPort());
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
      socket->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));
      ESB_LOG_DEBUG("Not returning connection to '%s:%d' to pool", buffer,
                   socket->getPeerAddress()->getPort());
    }

    socket->close();  // idempotent, just to be safe
    _embeddedList.addLast(socket);
    return;
  }

  ESB::EmbeddedList *list =
      (ESB::EmbeddedList *) _map.find(socket->getPeerAddress());

  if (list) {
    if (ESB_DEBUG_LOGGABLE) {
      char buffer[ESB_IPV6_PRESENTATION_SIZE];
      socket->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));
      ESB_LOG_DEBUG("Returning connection to '%s:%d' to pool", buffer,
                   socket->getPeerAddress()->getPort());
    }

    list->addLast(socket);
    return;
  }

  ESB::SocketAddress *address =
      new(&_allocator) ESB::SocketAddress(*socket->getPeerAddress());

  if (!address) {
    if (ESB_CRITICAL_LOGGABLE) {
      char buffer[ESB_IPV6_PRESENTATION_SIZE];
      socket->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));
      ESB_LOG_CRITICAL("Cannot return connection to '%s:%d' to pool: bad alloc",
                       buffer, socket->getPeerAddress()->getPort());
    }

    socket->close();
    _embeddedList.addLast(socket);
    return;
  }

  list = new(&_allocator) ESB::EmbeddedList();

  if (0 == list) {
    if (ESB_WARNING_LOGGABLE) {
      char buffer[ESB_IPV6_PRESENTATION_SIZE];
      socket->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));
      ESB_LOG_WARNING("Cannot return connection to '%s:%d' to pool: bad alloc",
                      buffer, socket->getPeerAddress()->getPort());
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
      socket->getPeerAddress()->getIPAddress(dottedIP, sizeof(dottedIP));
      ESB_LOG_ERRNO_WARNING(error, "Cannot return connection to '%s:%d' to pool",
                   dottedIP, socket->getPeerAddress()->getPort());
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
  _factory->release((HttpClientSocket *) object);
}

}  // namespace ES
