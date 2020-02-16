/* Copyright (c) 2011 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_CLIENT_SOCKET_FACTORY_H
#include <AWSHttpClientSocketFactory.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_FIXED_ALLOCATOR_H
#include <ESFFixedAllocator.h>
#endif

#ifndef ESF_NULL_LOGGER_H
#include <ESFNullLogger.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifndef ESF_NULL_LOCK_H
#include <ESFNullLock.h>
#endif

#ifndef ESF_WRITE_SCOPE_LOCK_H
#include <ESFWriteScopeLock.h>
#endif

class AddressComparator : public ESFComparator {
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
  ESFSocketAddress *address1 = (ESFSocketAddress *)first;
  ESFSocketAddress *address2 = (ESFSocketAddress *)second;

  ESF_ASSERT(address1);
  ESF_ASSERT(address2);

  return memcmp(address1->getAddress(), address2->getAddress(),
                sizeof(*address1->getAddress()));
}

static AddressComparator AddressComparator;

AWSHttpClientSocketFactory::AWSHttpClientSocketFactory(
    AWSHttpClientCounters *clientCounters, ESFLogger *logger)
    : _logger(logger ? logger : ESFNullLogger::GetInstance()),
      _clientCounters(clientCounters),
      _allocator(),
      _map(true, &AddressComparator,
           &_allocator,  // Will result in a leak if erase(), clear(), or
                         // insert() with a uniqueness violation
           ESFNullLock::Instance()),
      _embeddedList(),
      _mutex(),
      _cleanupHandler(this) {}

ESFError AWSHttpClientSocketFactory::initialize() {
  return _allocator.initialize(
      ESF_WORD_ALIGN(sizeof(AWSHttpClientSocket)) * 1000,
      ESFSystemAllocator::GetInstance());
}

void AWSHttpClientSocketFactory::destroy() {
  AWSHttpClientSocket *socket =
      (AWSHttpClientSocket *)_embeddedList.removeFirst();

  while (socket) {
    socket->~AWSHttpClientSocket();

    socket = (AWSHttpClientSocket *)_embeddedList.removeFirst();
  }

  ESFSocketAddress *address = 0;

  for (ESFMapIterator iterator = _map.getMinimumIterator();
       false == iterator.isNull(); iterator = iterator.getNext()) {
    address = (ESFSocketAddress *)iterator.getKey();

    ESF_ASSERT(address);

    address->~ESFSocketAddress();

    socket = (AWSHttpClientSocket *)iterator.getValue();

    ESF_ASSERT(socket);

    socket->~AWSHttpClientSocket();
  }

  _allocator.destroy();
}

AWSHttpClientSocketFactory::~AWSHttpClientSocketFactory() {}

AWSHttpClientSocket *AWSHttpClientSocketFactory::create(
    AWSHttpConnectionPool *pool, AWSHttpClientTransaction *transaction) {
  AWSHttpClientSocket *socket = 0;

  {
    ESFWriteScopeLock scopeLock(_mutex);

    ESFEmbeddedList *list =
        (ESFEmbeddedList *)_map.find(transaction->getPeerAddress());

    if (list) {
      socket = (AWSHttpClientSocket *)list->removeLast();

      if (socket) {
        if (_logger->isLoggable(ESFLogger::Debug)) {
          char buffer[16];

          transaction->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));

          _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                       "[pool] Reusing connection for %s:%d", buffer,
                       transaction->getPeerAddress()->getPort());
        }

        ESF_ASSERT(socket->isConnected());

        socket->reset(true, pool, transaction);

        return socket;
      }
    }

    if (_logger->isLoggable(ESFLogger::Debug)) {
      char buffer[16];

      transaction->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));

      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[pool] Creating new connection for %s:%d", buffer,
                   transaction->getPeerAddress()->getPort());
    }

    // Try to reuse a dead socket

    socket = (AWSHttpClientSocket *)_embeddedList.removeLast();
  }

  if (0 == socket) {
    // Failing all else, allocate a new socket

    socket = new (&_allocator) AWSHttpClientSocket(
        pool, transaction, _clientCounters, &_cleanupHandler, _logger);

    if (0 == socket) {
      if (_logger->isLoggable(ESFLogger::Critical)) {
        char buffer[16];

        transaction->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));

        _logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                     "[pool] Cannot allocate new connection for %s:%d", buffer,
                     transaction->getPeerAddress()->getPort());
      }

      return 0;
    }

    return socket;
  }

  ESF_ASSERT(false == socket->isConnected());

  socket->reset(false, pool, transaction);

  return socket;
}

void AWSHttpClientSocketFactory::release(AWSHttpClientSocket *socket) {
  if (!socket) {
    return;
  }

  ESFWriteScopeLock scopeLock(_mutex);

  if (false == socket->isConnected()) {
    if (_logger->isLoggable(ESFLogger::Debug)) {
      char buffer[16];

      socket->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));

      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[pool] Not returning connection for %s:%d to pool", buffer,
                   socket->getPeerAddress()->getPort());
    }

    socket->close();  // idempotent, just to be safe

    _embeddedList.addLast(socket);

    return;
  }

  ESFEmbeddedList *list =
      (ESFEmbeddedList *)_map.find(socket->getPeerAddress());

  if (list) {
    if (_logger->isLoggable(ESFLogger::Debug)) {
      char buffer[16];

      socket->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));

      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[pool] Returning connection for %s:%d to pool", buffer,
                   socket->getPeerAddress()->getPort());
    }

    list->addLast(socket);

    return;
  }

  ESFSocketAddress *address =
      new (&_allocator) ESFSocketAddress(*socket->getPeerAddress());

  if (0 == address) {
    if (_logger->isLoggable(ESFLogger::Warning)) {
      char buffer[16];

      socket->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));

      _logger->log(
          ESFLogger::Critical, __FILE__, __LINE__,
          "[pool] Cannot return connection for %s:%d to pool: bad alloc",
          buffer, socket->getPeerAddress()->getPort());
    }

    socket->close();

    _embeddedList.addLast(socket);

    return;
  }

  list = new (&_allocator) ESFEmbeddedList();

  if (0 == list) {
    if (_logger->isLoggable(ESFLogger::Warning)) {
      char buffer[16];

      socket->getPeerAddress()->getIPAddress(buffer, sizeof(buffer));

      _logger->log(
          ESFLogger::Critical, __FILE__, __LINE__,
          "[pool] Cannot return connection for %s:%d to pool: bad alloc",
          buffer, socket->getPeerAddress()->getPort());
    }

    socket->close();

    _embeddedList.addLast(socket);

    return;
  }

  list->addLast(socket);

  ESFError error = _map.insert(address, list);

  if (ESF_SUCCESS != error) {
    char buffer[1024];

    ESFDescribeError(error, buffer, sizeof(buffer));

    if (_logger->isLoggable(ESFLogger::Warning)) {
      char dottedIP[16];

      socket->getPeerAddress()->getIPAddress(dottedIP, sizeof(dottedIP));

      _logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                   "[pool] Cannot return connection for %s:%d to pool: %s",
                   dottedIP, socket->getPeerAddress()->getPort(), buffer);
    }

    socket->close();

    _embeddedList.addLast(socket);
  }
}

AWSHttpClientSocketFactory::CleanupHandler::CleanupHandler(
    AWSHttpClientSocketFactory *factory)
    : ESFCleanupHandler(), _factory(factory) {}

AWSHttpClientSocketFactory::CleanupHandler::~CleanupHandler() {}

void AWSHttpClientSocketFactory::CleanupHandler::destroy(ESFObject *object) {
  _factory->release((AWSHttpClientSocket *)object);
}
