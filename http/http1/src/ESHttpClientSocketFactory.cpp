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
    ESB::SocketMultiplexer &multiplexer, HttpClientHandler &handler,
    HttpClientCounters &counters, ESB::Allocator &allocator)
    : _multiplexer(multiplexer),
      _handler(handler),
      _counters(counters),
      _allocator(allocator),
      _map(AddressComparator),  // TODO replace with connection pool
      _cleanupHandler(*this),
      _dnsClient(),
      _ioBufferPoolAllocator(ESB::SystemConfig::Instance().pageSize() * 1000,
                             ESB::SystemConfig::Instance().cacheLineSize()),
      _ioBufferPool(ESB::SystemConfig::Instance().pageSize() -
                    ESB_ALIGN(ESB::SystemConfig::Instance().cacheLineSize(),
                              sizeof(ESB::Buffer))) {}

HttpClientSocketFactory::~HttpClientSocketFactory() {
  HttpClientSocket *socket = (HttpClientSocket *)_sockets.removeFirst();

  while (socket) {
    socket->~HttpClientSocket();
    socket = (HttpClientSocket *)_sockets.removeFirst();
  }

  ESB::SocketAddress *address = NULL;

  for (ESB::MapIterator iterator = _map.minimumIterator(); !iterator.isNull();
       iterator = iterator.next()) {
    address = (ESB::SocketAddress *)iterator.key();
    assert(address);
    address->~SocketAddress();
    socket = (HttpClientSocket *)iterator.value();
    assert(socket);
    socket->~HttpClientSocket();
  }
}

HttpClientSocket *HttpClientSocketFactory::create(
    HttpClientTransaction *transaction) {
  if (!transaction || !_clientStack) {
    return NULL;
  }

  HttpClientSocket *socket = NULL;
  ESB::EmbeddedList *list =
      (ESB::EmbeddedList *)_map.find(&transaction->peerAddress());

  if (list) {
    socket = (HttpClientSocket *)list->removeLast();

    if (socket) {
      if (ESB_DEBUG_LOGGABLE) {
        char buffer[ESB_IPV6_PRESENTATION_SIZE];
        transaction->peerAddress().presentationAddress(buffer, sizeof(buffer));
        ESB_LOG_DEBUG("Reusing connection for '%s:%d'", buffer,
                      transaction->peerAddress().port());
      }

      assert(socket->isConnected());
      socket->reset(true, transaction);
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
  socket = (HttpClientSocket *)_sockets.removeLast();

  if (socket) {
    assert(!socket->isConnected());
    socket->reset(false, transaction);
    return socket;
  }

  socket = new (_allocator)
      HttpClientSocket(_handler, *_clientStack, transaction, &_counters,
                       &_cleanupHandler, _ioBufferPool);

  if (!socket && ESB_CRITICAL_LOGGABLE) {
    char dottedIP[ESB_IPV6_PRESENTATION_SIZE];
    transaction->peerAddress().presentationAddress(dottedIP, sizeof(dottedIP));
    ESB_LOG_CRITICAL("Cannot allocate new connection for '%s:%d'", dottedIP,
                     transaction->peerAddress().port());
  }

  return socket;
}

void HttpClientSocketFactory::release(HttpClientSocket *socket) {
  if (!socket) {
    return;
  }

  if (!socket->isConnected()) {
    if (ESB_DEBUG_LOGGABLE) {
      char buffer[ESB_IPV6_PRESENTATION_SIZE];
      socket->peerAddress()->presentationAddress(buffer, sizeof(buffer));
      ESB_LOG_DEBUG("Not returning connection to '%s:%d' to pool", buffer,
                    socket->peerAddress()->port());
    }

    socket->close();  // idempotent, just to be safe
    _sockets.addLast(socket);
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
    _sockets.addLast(socket);
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
    _sockets.addLast(socket);
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
      ESB_LOG_WARNING_ERRNO(error, "Cannot connect to %s:%d", hostname, port);
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
    socket->close();
    // transaction->getHandler()->end(transaction,
    //                               HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot add client socket to multiplexer");
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
