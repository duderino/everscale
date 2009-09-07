/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_CLIENT_SOCKET_H
#include <AWSClientSocket.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifndef ESF_NULL_LOGGER_H
#include <ESFNullLogger.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_BUFFER_POOL_H
#include <ESFBufferPool.h>
#endif

#ifndef ESF_THREAD_H
#include <ESFThread.h>
#endif

// TODO buffer pool size not hardcoded
bool AWSClientSocket::_ReuseConnections = true;
ESFBufferPool AWSClientSocket::_BufferPool(1024, 900, ESFSystemAllocator::GetInstance());

AWSClientSocket::AWSClientSocket(AWSClientSocketFactory *factory,
                                 AWSPerformanceCounter *counter,
                                 int requestsPerConnection,
                                 const ESFSocketAddress &peer,
                                 ESFCleanupHandler *cleanupHandler,
                                 ESFLogger *logger) :
    _requestsPerConnection(requestsPerConnection),
    _inReadMode(false),
    _successCounter(counter),
    _logger(logger ? logger : ESFNullLogger::GetInstance()),
    _cleanupHandler(cleanupHandler),
    _buffer(0),
    _socket(peer, false),
    _factory(factory)
{
    setupBuffer();
    AWSPerformanceCounter::GetTime(&_start);
}

AWSClientSocket::~AWSClientSocket()
{
}

bool AWSClientSocket::wantAccept()
{
    return false;
}

bool AWSClientSocket::wantConnect()
{
    return false == _socket.isConnected();
}

bool AWSClientSocket::wantRead()
{
    if (0 == _buffer && false == setupBuffer())
    {
        return false;
    }

    return _socket.isConnected() && _inReadMode && 0 < _buffer->getWritable();
}

bool AWSClientSocket::wantWrite()
{
    if (0 == _buffer && false == setupBuffer())
    {
        return false;
    }

    return _socket.isConnected() && false == _inReadMode && 0 < _buffer->getReadable();
}

bool AWSClientSocket::isIdle()  // todo pass in current time to reduce number of syscalls
{
    return false;   // todo - implement
}

bool AWSClientSocket::handleAcceptEvent(ESFFlag *isRunning, ESFLogger *logger)
{
    if (_logger->isLoggable(ESFLogger::Warning))
    {
        _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                     "[socket:%d] Cannot handle accept events",
                     _socket.getSocketDescriptor());
    }

    return true;    // todo - keep in multiplexer
}

bool AWSClientSocket::handleConnectEvent(ESFFlag *isRunning, ESFLogger *logger)
{
    if (0 == _buffer && false == setupBuffer())
    {
        return false;
    }

    ESF_ASSERT(_socket.isConnected());

    if (_logger->isLoggable(ESFLogger::Debug))
    {
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[socket:%d] Connected",
                     _socket.getSocketDescriptor());
    }

    ESF_ASSERT(wantWrite());

    return handleWritableEvent(isRunning, logger);
}

bool AWSClientSocket::handleReadableEvent(ESFFlag *isRunning, ESFLogger *logger)
{
    if (0 == _buffer && false == setupBuffer())
    {
        return false;
    }

    ESF_ASSERT(_buffer);
    ESF_ASSERT(_inReadMode);
    ESF_ASSERT(_buffer->isWritable());

    ESFSSize result = 0;
    ESFError error = ESF_SUCCESS;

    while (isRunning->get() && _buffer->isWritable())
    {
        result = _socket.receive(_buffer);

        if (0 > result)
        {
            error = ESFGetLastError();

            if (ESF_AGAIN == error)
            {
                if (_logger->isLoggable(ESFLogger::Debug))
                {
                    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                                 "[socket:%d] not ready for read",
                                 _socket.getSocketDescriptor());
                }

                return true;    // keep in multiplexer
            }

            if (ESF_INTR == error)
            {
                if (_logger->isLoggable(ESFLogger::Debug))
                {
                    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                                 "[socket:%d] interrupted",
                                 _socket.getSocketDescriptor());
                }

                continue;
            }

            return handleErrorEvent(error, isRunning, logger);
        }

        if (0 == result)
        {
            return handleEndOfFileEvent(isRunning, logger);
        }

        if (_logger->isLoggable(ESFLogger::Debug))
        {
            _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                         "[socket:%d] Read %d bytes",
                         _socket.getSocketDescriptor(), result);
        }
    }

    if (! isRunning->get())
    {
        return false;   // remove from multiplexer
    }

    _successCounter->addObservation(&_start);

    _start.tv_usec = 0;
    _start.tv_sec = 0;

    if (_logger->isLoggable(ESFLogger::Debug))
    {
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[socket:%d] Received complete response",
                     _socket.getSocketDescriptor());
    }

    _inReadMode = false;

    ESF_ASSERT(wantWrite());

    if (false == _ReuseConnections)
    {
        return false;   // remove from multiplexer
    }

    AWSPerformanceCounter::GetTime(&_start);

    return true;    // keep in multiplexer - also yields to other sockets
}

bool AWSClientSocket::handleWritableEvent(ESFFlag *isRunning, ESFLogger *logger)
{
    if (0 == _buffer && false == setupBuffer())
    {
        return false;
    }

    ESF_ASSERT(_buffer);
    ESF_ASSERT(false == _inReadMode);
    ESF_ASSERT(_buffer->isReadable());

    ESFSSize result = 0;
    ESFError error = ESF_SUCCESS;

    while (isRunning->get() && _buffer->isReadable())
    {
        result = _socket.send(_buffer);

        if (0 > result)
        {
            error = ESFGetLastError();

            if (ESF_AGAIN == error)
            {
                if (_logger->isLoggable(ESFLogger::Debug))
                {
                    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                                 "[socket:%d] Not ready for write",
                                 _socket.getSocketDescriptor());
                }

                return true;    // keep in multiplexer
            }

            if (ESF_INTR == error)
            {
                if (_logger->isLoggable(ESFLogger::Debug))
                {
                    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                                 "[socket:%d] Interrupted",
                                 _socket.getSocketDescriptor());
                }

                continue;
            }

            return handleErrorEvent(error, isRunning, logger);
        }

        if (_logger->isLoggable(ESFLogger::Debug))
        {
            _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                         "[socket:%d] Wrote %d bytes",
                         _socket.getSocketDescriptor(), result);
        }
    }

    if (! isRunning->get())
    {
        return false;   // remove from multiplexer
    }

    if (_logger->isLoggable(ESFLogger::Debug))
    {
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[socket:%d] Sent complete request",
                     _socket.getSocketDescriptor());
    }

    _inReadMode = true;

    ESF_ASSERT(false == wantRead());

    _buffer->compact();

    ESF_ASSERT(true == wantRead());

    return handleReadableEvent(isRunning, logger);
}

bool AWSClientSocket::handleErrorEvent(ESFError errorCode, ESFFlag *isRunning, ESFLogger *logger)
{
    if (_logger->isLoggable(ESFLogger::Warning))
    {
        char buffer[100];
        char dottedAddress[16];

        _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
        ESFDescribeError(errorCode, buffer, sizeof(buffer));

        _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                     "[socket:%d] Error from server %s: %s",
                     _socket.getSocketDescriptor(), dottedAddress, buffer);
    }

    return false;    // remove from multiplexer
}

bool AWSClientSocket::handleEndOfFileEvent(ESFFlag *isRunning, ESFLogger *logger)
{
    if (_logger->isLoggable(ESFLogger::Debug))
    {
        char dottedAddress[16];

        _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));

        _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                     "[socket:%d] Server %s closed socket",
                     _socket.getSocketDescriptor(), dottedAddress);
    }

    return false;    // remove from multiplexer
}

bool AWSClientSocket::handleIdleEvent(ESFFlag *isRunning, ESFLogger *logger)
{
    if (_logger->isLoggable(ESFLogger::Debug))
    {
        char dottedAddress[16];

        _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));

        _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                     "[socket:%d] Server %s is idle",
                     _socket.getSocketDescriptor(), dottedAddress);
    }

    return false;    // remove from multiplexer
}

bool AWSClientSocket::handleRemoveEvent(ESFFlag *isRunning, ESFLogger *logger)
{
    if (_logger->isLoggable(ESFLogger::Debug))
    {
        char dottedAddress[16];

        _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));

        _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                     "[socket:%d] Closing socket for server %s",
                     _socket.getSocketDescriptor(), dottedAddress);
    }

    _BufferPool.releaseBuffer(_buffer);
    _buffer = 0;
    _socket.close();

    if (false == isRunning->get())
    {
        return true;    // call cleanup handler on us after this returns
    }

    ESFError error = _factory->addNewConnection(_socket.getPeerAddress());

    if (ESF_SUCCESS != error)
    {
        if (logger->isLoggable(ESFLogger::Critical))
        {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                        "[socket] Cannot add new connection: %s", buffer);
        }
    }

    return true;    // call cleanup handler on us after this returns
}

SOCKET AWSClientSocket::getSocketDescriptor() const
{
    return _socket.getSocketDescriptor();
}

ESFCleanupHandler *AWSClientSocket::getCleanupHandler()
{
    return _cleanupHandler;
}

const char *AWSClientSocket::getName() const
{
    return "AWSClientSocket";
}

bool AWSClientSocket::run(ESFFlag *isRunning)
{
    return false;   // todo - log warning
}

bool AWSClientSocket::setupBuffer()
{
    if (_buffer)
    {
        return true;
    }

    _buffer = _BufferPool.acquireBuffer();

    if (! _buffer)
    {
        if (_logger->isLoggable(ESFLogger::Error))
        {
            _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                         "[socket:%d] Cannot acquire new buffer",
                         _socket.getSocketDescriptor());
        }

        return false;
    }

    ESF_ASSERT(_buffer->isWritable());

    _buffer->clear();

    unsigned int capacity = _buffer->getCapacity();

    memset(_buffer->getBuffer(), ESFThread::GetThreadId() % 256, capacity);

    _buffer->setWritePosition(capacity);

    _inReadMode = false;

    ESF_ASSERT(capacity = _buffer->getReadable());
    ESF_ASSERT(0 == _buffer->getWritable());

    return true;
}


