/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

v#ifndef AWS_CLIENT_SOCKET_FACTORY_H
#include <AWSClientSocketFactory.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef AWS_CLIENT_SOCKET_H
#include <AWSClientSocket.h>
#endif

AWSClientSocketFactory::AWSClientSocketFactory(int maxSockets,
                                               AWSPerformanceCounter *successCounter,
                                               ESFSocketMultiplexerDispatcher *dispatcher,
                                               ESFLogger *logger) :
    _dispatcher(dispatcher),
    _successCounter(successCounter),
    _logger(logger),
    _fixedAllocator(maxSockets + 10, sizeof(AWSClientSocket), ESFSystemAllocator::GetInstance()),
    _sharedAllocator(&_fixedAllocator),
    _cleanupHandler(&_sharedAllocator)
{
}

AWSClientSocketFactory::~AWSClientSocketFactory()
{
}

ESFError AWSClientSocketFactory::initialize()
{
    return _sharedAllocator.initialize();
}

#include <errno.h>

ESFError AWSClientSocketFactory::addNewConnection(const ESFSocketAddress &address)
{
    AWSClientSocket *socket = new(&_sharedAllocator) AWSClientSocket(this,
                                                                     _successCounter,
                                                                     -1,
                                                                     address,
                                                                     &_cleanupHandler,
                                                                     _logger);

    if (! socket)
    {
        if (_logger->isLoggable(ESFLogger::Critical))
        {
            _logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                        "[factory] Cannot allocate new client socket");
        }

        return ESF_OUT_OF_MEMORY;
    }

    ESFError error = ESF_SUCCESS;

    while (true)
    {
        error = socket->connect();

        if (EADDRNOTAVAIL == errno)
        {
            if (_logger->isLoggable(ESFLogger::Warning))
            {
                _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                             "[factory] Spurious EADDRNOTAVAIL on connect");
            }

            continue;
        }

        break;
    }

    if (ESF_SUCCESS != error)
    {
        if (_logger->isLoggable(ESFLogger::Critical))
        {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            _logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                        "[factory] Cannot connect to peer: %s", buffer);
        }

        return error;
    }

    error = _dispatcher->addMultiplexedSocket(socket);

    if (ESF_SUCCESS != error)
    {
        if (_logger->isLoggable(ESFLogger::Critical))
        {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            _logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                        "[factory] Cannot add client socket to multiplexer: %s", buffer);
        }

        return error;
    }

    return ESF_SUCCESS;
}


