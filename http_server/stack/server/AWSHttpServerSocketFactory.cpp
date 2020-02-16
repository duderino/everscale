/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_SERVER_SOCKET_FACTORY_H
#include <AWSHttpServerSocketFactory.h>
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

AWSHttpServerSocketFactory::AWSHttpServerSocketFactory(
    AWSHttpServerCounters *counters, ESFLogger *logger)
    : _logger(logger ? logger : ESFNullLogger::GetInstance()),
      _counters(counters),
      _allocator(),
      _embeddedList(),
      _mutex(),
      _cleanupHandler(this) {}

ESFError AWSHttpServerSocketFactory::initialize() {
  return _allocator.initialize(
      ESF_WORD_ALIGN(sizeof(AWSHttpServerSocket)) * 1000,
      ESFSystemAllocator::GetInstance());
}

void AWSHttpServerSocketFactory::destroy() {
  AWSHttpServerSocket *socket =
      (AWSHttpServerSocket *)_embeddedList.removeFirst();

  while (socket) {
    socket->~AWSHttpServerSocket();

    socket = (AWSHttpServerSocket *)_embeddedList.removeFirst();
  }

  _allocator.destroy();
}

AWSHttpServerSocketFactory::~AWSHttpServerSocketFactory() {}

AWSHttpServerSocket *AWSHttpServerSocketFactory::create(
    AWSHttpServerHandler *handler, ESFTCPSocket::AcceptData *acceptData) {
  AWSHttpServerSocket *socket = 0;

  _mutex.writeAcquire();

  socket = (AWSHttpServerSocket *)_embeddedList.removeLast();

  _mutex.writeRelease();

  if (0 == socket) {
    socket = new (&_allocator)
        AWSHttpServerSocket(handler, &_cleanupHandler, _logger, _counters);

    if (0 == socket) {
      return 0;
    }
  }

  socket->reset(handler, acceptData);

  return socket;
}

void AWSHttpServerSocketFactory::release(AWSHttpServerSocket *socket) {
  if (!socket) {
    return;
  }

  _mutex.writeAcquire();

  _embeddedList.addLast(socket);

  _mutex.writeRelease();
}

AWSHttpServerSocketFactory::CleanupHandler::CleanupHandler(
    AWSHttpServerSocketFactory *factory)
    : ESFCleanupHandler(), _factory(factory) {}

AWSHttpServerSocketFactory::CleanupHandler::~CleanupHandler() {}

void AWSHttpServerSocketFactory::CleanupHandler::destroy(ESFObject *object) {
  _factory->release((AWSHttpServerSocket *)object);
}
