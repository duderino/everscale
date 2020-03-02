#ifndef ES_HTTP_CLIENT_SOCKET_FACTORY_H
#include <ESHttpClientSocketFactory.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_FIXED_ALLOCATOR_H
#include <ESBFixedAllocator.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
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
  ESB::SocketAddress *address1 = (ESB::SocketAddress *)first;
  ESB::SocketAddress *address2 = (ESB::SocketAddress *)second;

  assert(address1);
  assert(address2);

  return memcmp(address1->getAddress(), address2->getAddress(),
                sizeof(*address1->getAddress()));
}

static AddressComparator AddressComparator;

HttpClientSocketFactory::HttpClientSocketFactory(
    HttpClientCounters *clientCounters, ESB::Logger *logger)
    : _logger(logger ? logger : ESB::NullLogger::GetInstance()),
      _clientCounters(clientCounters),
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
  HttpClientSocket *socket = (HttpClientSocket *)_embeddedList.removeFirst();

  while (socket) {
    socket->~HttpClientSocket();
    socket = (HttpClientSocket *)_embeddedList.removeFirst();
  }

  ESB::SocketAddress *address = 0;

  for (ESB::MapIterator iterator = _map.getMinimumIterator();
       false == iterator.isNull(); iterator = iterator.getNext()) {
    address = (ESB::SocketAddress *)iterator.getKey();

    assert(address);

    address->~SocketAddress();

    socket = (HttpClientSocket *)iterator.getValue();

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
        (ESB::EmbeddedList *)_map.find(transaction->getPeerAddress());

    if (list) {
      socket = (HttpClientSocket *)list->removeLast();

      if (socket) {
        if (_logger->isLoggable(ESB::Logger::Debug)) {
          char buffer[16];

          transaction->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));

          _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                       "[pool] Reusing connection for %s:%d", buffer,
                       transaction->getPeerAddress()->getPort());
        }

        assert(socket->isConnected());

        socket->reset(true, pool, transaction);

        return socket;
      }
    }

    if (_logger->isLoggable(ESB::Logger::Debug)) {
      char buffer[16];

      transaction->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[pool] Creating new connection for %s:%d", buffer,
                   transaction->getPeerAddress()->getPort());
    }

    // Try to reuse a dead socket

    socket = (HttpClientSocket *)_embeddedList.removeLast();
  }

  if (0 == socket) {
    // Failing all else, allocate a new socket

    socket = new (&_allocator) HttpClientSocket(
        pool, transaction, _clientCounters, &_cleanupHandler, _logger);

    if (0 == socket) {
      if (_logger->isLoggable(ESB::Logger::Critical)) {
        char buffer[16];

        transaction->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));

        _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                     "[pool] Cannot allocate new connection for %s:%d", buffer,
                     transaction->getPeerAddress()->getPort());
      }

      return 0;
    }

    return socket;
  }

  assert(false == socket->isConnected());

  socket->reset(false, pool, transaction);

  return socket;
}

void HttpClientSocketFactory::release(HttpClientSocket *socket) {
  if (!socket) {
    return;
  }

  ESB::WriteScopeLock scopeLock(_mutex);

  if (false == socket->isConnected()) {
    if (_logger->isLoggable(ESB::Logger::Debug)) {
      char buffer[16];

      socket->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[pool] Not returning connection for %s:%d to pool", buffer,
                   socket->getPeerAddress()->getPort());
    }

    socket->close();  // idempotent, just to be safe

    _embeddedList.addLast(socket);

    return;
  }

  ESB::EmbeddedList *list =
      (ESB::EmbeddedList *)_map.find(socket->getPeerAddress());

  if (list) {
    if (_logger->isLoggable(ESB::Logger::Debug)) {
      char buffer[16];

      socket->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[pool] Returning connection for %s:%d to pool", buffer,
                   socket->getPeerAddress()->getPort());
    }

    list->addLast(socket);

    return;
  }

  ESB::SocketAddress *address =
      new (&_allocator) ESB::SocketAddress(*socket->getPeerAddress());

  if (0 == address) {
    if (_logger->isLoggable(ESB::Logger::Warning)) {
      char buffer[16];

      socket->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));

      _logger->log(
          ESB::Logger::Critical, __FILE__, __LINE__,
          "[pool] Cannot return connection for %s:%d to pool: bad alloc",
          buffer, socket->getPeerAddress()->getPort());
    }

    socket->close();

    _embeddedList.addLast(socket);

    return;
  }

  list = new (&_allocator) ESB::EmbeddedList();

  if (0 == list) {
    if (_logger->isLoggable(ESB::Logger::Warning)) {
      char buffer[16];

      socket->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));

      _logger->log(
          ESB::Logger::Critical, __FILE__, __LINE__,
          "[pool] Cannot return connection for %s:%d to pool: bad alloc",
          buffer, socket->getPeerAddress()->getPort());
    }

    socket->close();

    _embeddedList.addLast(socket);

    return;
  }

  list->addLast(socket);

  ESB::Error error = _map.insert(address, list);

  if (ESB_SUCCESS != error) {
    char buffer[1024];

    ESB::DescribeError(error, buffer, sizeof(buffer));

    if (_logger->isLoggable(ESB::Logger::Warning)) {
      char dottedIP[16];

      socket->getPeerAddress()->getIPAddress(dottedIP, sizeof(dottedIP));

      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[pool] Cannot return connection for %s:%d to pool: %s",
                   dottedIP, socket->getPeerAddress()->getPort(), buffer);
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
