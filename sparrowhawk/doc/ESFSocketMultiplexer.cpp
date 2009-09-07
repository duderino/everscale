/** @file ESFSocketMultiplexer.cpp
 *  @brief An object that allows multiple sockets to be handled by a single
 *      thread
 *
 *  Copyright 2005 Joshua Blatt
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:05 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

todo remove

#ifndef ESF_SOCKET_MULTIPLEXER_H
#include <ESFSocketMultiplexer.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifndef ESF_WRITE_SCOPE_LOCK_H
#include <ESFWriteScopeLock.h>
#endif

#ifndef ESF_NULL_LOCK_H
#include <ESFNullLock.h>
#endif

#ifndef ESF_ERROR_H
#include <ESFError.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

/** @todo make this file more portable - esp when you do the win32 port */

#ifndef HAVE_SELECT
#error "select is the basis of this file, equivalent is needed"
#endif

long ESFSocketMultiplexer::_SelectTimeout = 500;
long ESFSocketMultiplexer::_ConnectedSocketTimeout = 60;
long ESFSocketMultiplexer::_ConnectingSocketTimeout = 60;
long ESFSocketMultiplexer::_ListeningSocketTimeout = 60 * 60;

ESFMultiplexedSocket::ESFMultiplexedSocket() :
    _lastAccess( ESFDate::GetSystemTime() )
{
}

ESFMultiplexedSocket::~ESFMultiplexedSocket()
{
}

void
ESFMultiplexedSocket::setLastAccess( const ESFDate &date )
{
    _lastAccess = date;
}

const ESFDate &
ESFMultiplexedSocket::getLastAccess() const
{
    return _lastAccess;
}

ESFSocketMultiplexer::ESFSocketMultiplexer( ESFAllocator *allocator,
                                            ESFLogger *logger ) :
    _mutex(),
    _pendingSockets( allocator, ESFNullLock::Instance() ),
    _activeSockets( allocator, ESFNullLock::Instance() ),
    _logger( logger )
{
}

ESFSocketMultiplexer::~ESFSocketMultiplexer()
{
}

bool
ESFSocketMultiplexer::addSocket( ESFMultiplexedSocket *socket )
{
    ESFWriteScopeLock scopeLock( _mutex );

    //
    //  since active sockets and pending sockets share the same allocator, if
    //  this succeeds, then there will be enough memory to store the socket
    //  in the active sockets list.
    //

    return _pendingSockets.pushBack( socket );
}

bool
ESFSocketMultiplexer::addSocketFromCallback( ESFMultiplexedSocket *socket )
{
    //
    //  This must be locked because the active socket list and the pending
    //  socket list share the same allocator.
    //
    ESFWriteScopeLock scopeLock( _mutex );

    return _activeSockets.pushBack( socket );
}

void
ESFSocketMultiplexer::run()
{
    int events = 0;
    ESFError error = ESF_SUCCESS;
    ESFListIterator it;
    ESFMultiplexedSocket *socket;
    fd_set readSet;
    fd_set writeSet;
    fd_set errorSet;
    SOCKET maxSocketDescriptor = INVALID_SOCKET;
    SOCKET socketDescriptor = INVALID_SOCKET;
    struct timeval timeout;
    ESFDate now;

    while ( false == isStopRequested() )
    {
        //
        //  Move all pending sockets to active list.
        //

        _mutex.writeAcquire();

        while ( ! _pendingSockets.isEmpty() )
        {
            socket = ( ESFMultiplexedSocket * ) _pendingSockets.getFront();
            _pendingSockets.popFront();

            if ( false == socket->handleAddEvent( *this, _logger ) )
            {
                socket->handleRemoveEvent( *this, _logger );
            }
            else
            {
                _activeSockets.pushBack( socket );
            }
        }

        _mutex.writeRelease();

        //
        //  Initialize for new run
        //

        FD_ZERO( &readSet );
        FD_ZERO( &writeSet );
        FD_ZERO( &errorSet );
        maxSocketDescriptor = 0;
        timeout.tv_sec = _SelectTimeout / 1000;
        timeout.tv_usec = ( _SelectTimeout % 1000 ) * 1000;

        //
        //  Populate the select sets.
        //

        for ( it = _activeSockets.getFrontIterator();
              ! it.isNull();
              it = it.getNext() )
        {
            socket = ( ESFMultiplexedSocket * ) it.getValue();
            socketDescriptor = socket->getSocketDescriptor();

            FD_SET( socketDescriptor, &errorSet );

            if ( socketDescriptor > maxSocketDescriptor )
            {
                maxSocketDescriptor = socketDescriptor;
            }

            if ( socket->isListeningSocket() )
            {
                FD_SET( socketDescriptor, &readSet );
                FD_SET( socketDescriptor, &writeSet );
            }
            else
            {
                if ( socket->isConnected() )
                {
                    if ( socket->registerForReadEvent() )
                    {
                        FD_SET( socketDescriptor, &readSet );
                    }

                    if ( socket->registerForWriteEvent() )
                    {
                        FD_SET( socketDescriptor, &writeSet );
                    }
                }
                else
                {
                    FD_SET( socketDescriptor, &readSet );
                    FD_SET( socketDescriptor, &writeSet );
                }
            }
        }

        //
        //  Select
        //

        events = select( maxSocketDescriptor + 1,
                         &readSet,
                         &writeSet,
                         &errorSet,
                         &timeout );


        if ( 0 > events )
        {
            error = ESFGetLastError();

            if ( ESF_INTR == error )
            {
                continue;
            }

            if ( _logger )
            {
                _logger->log( ESFLogger::Critical,
                              __FILE__,
                              __LINE__,
                              "select error: %d\n",
                              error );
            }

            shutdown();

            break;
        }

        //
        //  Dispatch all events
        //

        it = _activeSockets.getFrontIterator();

        now = ESFDate::GetSystemTime();

        while ( ! it.isNull() )
        {
            socket = ( ESFMultiplexedSocket * ) it.getValue();
            socketDescriptor = socket->getSocketDescriptor();

            // Listening sockets
            if ( socket->isListeningSocket() )
            {
                if ( FD_ISSET( socketDescriptor, &readSet ) &&
                     FD_ISSET( socketDescriptor, &writeSet ) )
                {
                    error = socket->getLastError();

                    if ( ESF_SUCCESS != error )
                    {
                        if ( ESF_AGAIN != error &&
                             false == socket->handleErrorEvent( *this,
                                                                error,
                                                                _logger ) )
                        {
                            it = removeSocket( it );
                            socket->handleRemoveEvent( *this, _logger );
                            continue;
                        }

                        it = it.getNext();
                        continue;
                    }
                }

                if ( FD_ISSET( socketDescriptor, &readSet ) &&
                     socket->registerForAcceptEvent() )
                {
                    if ( false == socket->handleAcceptEvent( *this, _logger ) )
                    {
                        it = removeSocket( it );
                        socket->handleRemoveEvent( *this, _logger );
                        continue;
                    }

                    socket->setLastAccess( now );
                }

                if ( socket->getLastAccess() + _ListeningSocketTimeout < now )
                {
                    if ( false == socket->handleTimeoutEvent( *this, _logger ) )
                    {
                        it = removeSocket( it );
                        socket->handleRemoveEvent( *this, _logger );
                        continue;
                    }
                }

                it = it.getNext();
                continue;
            }

            // Connected sockets
            if ( socket->isConnected() )
            {
                if ( FD_ISSET( socketDescriptor, &readSet ) &&
                     FD_ISSET( socketDescriptor, &writeSet ) )
                {
                    error = socket->getLastError();

                    if ( ESF_SUCCESS != error )
                    {
                        if ( ESF_AGAIN != error &&
                             false == socket->handleErrorEvent( *this,
                                                                error,
                                                                _logger ) )
                        {
                            it = removeSocket( it );
                            socket->handleRemoveEvent( *this, _logger );
                            continue;
                        }

                        it = it.getNext();
                        continue;
                    }

                    // Graceful close
                    if ( 1 > socket->getBytesReadable() )
                    {
                        if ( false == socket->handleErrorEvent( *this,
                                                                error,
                                                                _logger ) )
                        {
                            it = removeSocket( it );
                            socket->handleRemoveEvent( *this, _logger );
                            continue;
                        }

                        it = it.getNext();
                        continue;
                    }
                }

                if ( FD_ISSET( socketDescriptor, &writeSet ) &&
                               socket->registerForWriteEvent() )
                {
                    if ( false == socket->handleWriteEvent( *this, _logger ) )
                    {
                        it = removeSocket( it );
                        socket->handleRemoveEvent( *this, _logger );
                        continue;
                    }

                    socket->setLastAccess( now );
                }

                if ( FD_ISSET( socketDescriptor, &readSet ) &&
                     socket->registerForReadEvent() )
                {
                    if ( false == socket->handleReadEvent( *this, _logger ) )
                    {
                        it = removeSocket( it );
                        socket->handleRemoveEvent( *this, _logger );
                        continue;
                    }

                    socket->setLastAccess( now );
                }

                if ( socket->getLastAccess() + _ConnectedSocketTimeout < now )
                {
                    if ( false == socket->handleTimeoutEvent( *this, _logger ) )
                    {
                        it = removeSocket( it );
                        socket->handleRemoveEvent( *this, _logger );
                        continue;
                    }
                }

                it = it.getNext();
                continue;
            }

            // Connecting sockets
            if ( FD_ISSET( socketDescriptor, &readSet ) &&
                 1 > socket->getBytesReadable() )
            {
                error = ESFGetLastError();

                if ( ESF_AGAIN != error &&
                     false == socket->handleErrorEvent( *this,
                                                        error,
                                                        _logger ) )
                {
                    it = removeSocket( it );
                    socket->handleRemoveEvent( *this, _logger );
                    continue;
                }
            }
            else if ( FD_ISSET( socketDescriptor, &writeSet ) )
            {
                if ( socket->registerForConnectEvent() )
                {
                    if ( false == socket->handleConnectEvent( *this, _logger ) )
                    {
                        it = removeSocket( it );
                        socket->handleRemoveEvent( *this, _logger );
                        continue;
                    }
                }

                socket->setLastAccess( now );
            }
            else if ( now > socket->getLastAccess() + _ConnectingSocketTimeout )
            {
                if ( false == socket->handleTimeoutEvent( *this, _logger ) )
                {
                    it = removeSocket( it );
                    socket->handleRemoveEvent( *this, _logger );
                    continue;
                }
            }

            it = it.getNext();
        }
    }
}

ESFListIterator
ESFSocketMultiplexer::removeSocket( ESFListIterator &iterator )
{
    ESFListIterator next = iterator.getNext();

    _activeSockets.erase( &iterator );

    return next;
}

void
ESFSocketMultiplexer::shutdown()
{
    ESFListIterator it = _activeSockets.getFrontIterator();
    ESFMultiplexedSocket *socket;

    while ( ! it.isNull() )
    {
        socket = ( ESFMultiplexedSocket * ) it.getValue();
        it = removeSocket( it );
        socket->handleRemoveEvent( *this, _logger );
    }
}

ESFSize
ESFSocketMultiplexer::GetAllocationSize()
{
    return ESFList::GetAllocationSize();
}
