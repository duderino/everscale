/** @file ESFSocketMultiplexer.h
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
#define ESF_SOCKET_MULTIPLEXER_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_THREAD_H
#include <ESFThread.h>
#endif

#ifndef  ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_LIST_H
#include <ESFList.h>
#endif

#ifndef ESF_LOCKABLE_H
#include <ESFLockable.h>
#endif

#ifndef ESF_SOCKET_H
#include <ESFSocket.h>
#endif

#ifndef ESF_ERROR_H
#include <ESFError.h>
#endif

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

#ifndef ESF_DATE_H
#include <ESFDate.h>
#endif

class ESFSocketMultiplexer;

/** Any socket that can be managed by the ESFSocketMultiplexer must realize
 *  this interface.  This interface defines a set of callbacks that
 *  will be called by the ESFSocketMultiplexer whenever socket i/o events
 *  occur.
 *
 *  @ingroup network
 */
class ESFMultiplexedSocket
{
public:

    /** Default constructor */
    ESFMultiplexedSocket();

    /** Destructor */
    virtual ~ESFMultiplexedSocket();

    /** Determine whether this is a connected socket or a listening socket.
     *
     *  @return true if this is a listening socket, false otherwise.
     */
    virtual bool isListeningSocket() = 0;

    /** Determine whether or not this socket is currently connected.  Note that
     *  this is a logical state determined by the application.  A socket
     *  doing a non-blocking connect would always return false until the
     *  ESFSocketMultiplexer calls its handleConnectEvent methods.  In other
     *  words, an implementation should not make a system call to determine
     *  whether or not the socket is connected.
     *  <p>
     *  The socket multiplexer will only call this method if not a listening
     *  socket - if the isListeningSocket method returns false.
     *  </p>
     *
     *  @see isListeningSocket
     *  @return true if the socket is connected, false otherwise.
     */
    virtual bool isConnected() = 0;

    /** Get the socket descriptor for this socket.  The caller agrees not to
     *  read or write data from this socket.  The caller will use the file
     *  descriptor only to detect i/o events.
     *
     *  @return The multiplexed socket's socket descriptor
     */
    virtual SOCKET getSocketDescriptor() = 0;

    /** Get the number of bytes available to be read from this socket.  This
     *  should be implemented with a system call.
     *
     *  @return The number of bytes avaiable to be read from this socket.
     */
    virtual int getBytesReadable() = 0;

    /** Get and clear the last error on this socket.
     *
     *  @return the last error that occurred on this socket or ESF_SUCCESS if
     *      no error has occurred.
     */
    virtual ESFError getLastError() = 0;

    /** Update time of last access.  For connected sockets, this is the
     *  last time that data was read from or written to the socket.  For
     *  connecting sockets, this is the time either that the connect operation
     *  began or finished.  For listening sockets, this is the last time a
     *  new socket was accepted.
     *
     *  @param date The time of last access.
     */
    void setLastAccess( const ESFDate &date );

    /** Get the time of last access.  For connected sockets, this is the
     *  last time that data was read from or written to the socket.  For
     *  connecting sockets, this is the time either that the connect operation
     *  began or finished.  For listening sockets, this is the last time a
     *  new socket was accepted.
     *
     *  @return The time of last access
     */
    const ESFDate &getLastAccess() const;

    /** Called by the socket multiplexer when the multiplexer assumes control
     *  of the socket.
     *
     *  @param multiplexer The calling socket multiplexer.
     *  @param logger An optional logger instance.  May be null.
     *  @return true if successful, false otherwise.  A false return will
     *      tell the socket multiplexer to remove the socket from its list of
     *      multiplexed sockets.  The socket multiplexer will then call the
     *      handleRemoveEvent when it has removed the socket.
     */
    virtual bool handleAddEvent( ESFSocketMultiplexer &multiplexer,
                                 ESFLogger *logger ) = 0;

    /** Called by the socket multiplexer when the multiplexer is giving up
     *  control of the socket.  Sockets may choose to close themselves at this
     *  point, etc.
     *
     *  @param multiplexer The calling socket multiplexer.
     *  @param logger An optional logger instance.  May be null.
     */
    virtual void handleRemoveEvent( ESFSocketMultiplexer &multiplexer,
                                    ESFLogger *logger ) = 0;

    /** Called by the socket multiplexer whenever a socket times out.  The
     *  amount of time it takes a socket to time out varies by socket type
     *  (i.e., connected, connecting, listening) and can be configured through
     *  system settings.
     *
     *  @param multiplexer The calling socket multiplexer.
     *  @param logger An optional logger instance.  May be null.
     *  @return true if the socket does not want to be removed from the socket
     *      multiplexer (the socket can call updateLastAccess on itself to
     *      prevent repeated calls by the multiplexer), false otherwise.
     */
    virtual bool handleTimeoutEvent( ESFSocketMultiplexer &multiplexer,
                                     ESFLogger *logger ) = 0;

    /** Called by the socket multiplexer when there is data available for
     *  reading by the socket.  Only called for connected sockets in a
     *  connected state.
     *
     *  @param multiplexer The calling socket multiplexer.
     *  @param logger An optional logger instance.  May be null.
     *  @return true if successful, false otherwise.  A false return will
     *      tell the socket multiplexer to remove the socket from its list of
     *      multiplexed sockets.  The socket multiplexer will then call the
     *      handleRemoveEvent when it has removed the socket.
     */
    virtual bool handleReadEvent( ESFSocketMultiplexer &multiplexer,
                                  ESFLogger *logger ) = 0;

    /** Called by the socket multiplexer when there is space available for
     *  writing on the socket (when a write to the socket would not block).
     *  Only called for connected sockets in a connected state.
     *
     *  @param multiplexer The calling socket multiplexer.
     *  @param logger An optional logger instance.  May be null.
     *  @return true if successful, false otherwise.  A false return will tell
     *      the socket multiplexer to remove the socket from its list of
     *      multiplexed sockets.  The socket multiplexer will then call the
     *      handleRemoveEvent when it has removed the socket.
     */
    virtual bool handleWriteEvent( ESFSocketMultiplexer &multiplexer,
                                   ESFLogger *logger ) = 0;

    /** Called by the socket multiplexer when a connection has been established.
     *  Only called for connected (i.e., non-listening) sockets that have
     *  not connected yet.
     *
     *  @param multiplexer The calling socket multiplexer.
     *  @param logger An optional logger instance.  May be null.
     *  @return true if successful, false otherwise.  A false return will tell
     *      the socket multiplexer to remove the socket from its list of
     *      multiplexed sockets.   The socket multiplexer will then call the
     *      handleRemoveEvent when it has removed the socket.
     */
    virtual bool handleConnectEvent( ESFSocketMultiplexer &multiplexer,
                                     ESFLogger *logger ) = 0;

    /** Called by the socket multiplexer when either an error occurs on the
     *  socket or the socket's peer has closed its end of the socket.
     *
     *  @param multiplexer The calling socket multiplexer.
     *  @param logger An optional logger instance.  May be null.
     *  @param error The socket error code describing the error.
     *  @return true if the error could be remedied, false otherwise.  A false
     *      return will tell the socket multiplexer to remove the socket from
     *      its list of sockets.  The socket multiplexer will then call
     *      the handleRemoveEvent when it has removed the socket.
     */
    virtual bool handleErrorEvent( ESFSocketMultiplexer &multiplexer,
                                   ESFError error, ESFLogger *logger ) = 0;

    /** Called by the socket multiplexer when a new connection has been
     *  received.  Only called for a listening socket.
     *
     *  @param multiplexer The calling socket multiplexer.
     *  @param logger An optional logger instance.  May be null.
     *  @return true if successful, false otherwise.  A false return will tell
     *      the socket multiplexer to remove the socket from its list of
     *      multiplexed sockets.  The socket multiplexer will then call the
     *      handleRemoveEvent method when it has removed the socket.
     */
    virtual bool handleAcceptEvent( ESFSocketMultiplexer &multiplexer,
                                    ESFLogger *logger ) = 0;

    /** Called by the socket multiplexer to determine whether the socket
     *  implementation wants to be notified for read i/o events.  Only called
     *  for connected sockets that have an established connection.
     *
     *  @return true if the socket wants to be notified for read events, false
     *      otherwise.
     */
    virtual bool registerForReadEvent() = 0;

    /** Called by the socket multiplexer to determine whether the socket
     *  implementation wants to be notified for write i/o events.  Only called
     *  for connected sockets that have an established connection.
     *
     *  @return true if the socket wants to be notified for write events, false
     *      otherwise.
     */
    virtual bool registerForWriteEvent() = 0;

    /** Called by the socket multiplexer to determine whether the socket
     *  implementation wants to be notified for incoming connections.  Only
     *  called for listening sockets.
     *
     *  @return true if the socket wants to be notified for incoming
     *      connections, false otherwise.
     */
    virtual bool registerForAcceptEvent() = 0;

    /** Called by the socket multiplexer to determine whether the socket
     *  implementation wants to be notified for connect events.  Only called
     *  for connected sockets that do not have an established connection.
     *
     *  @return true if the socket wants to be notified when a connection
     *      is established.
     */
    virtual bool registerForConnectEvent() = 0;

private:

    ESFDate _lastAccess;
};

/** ESFSocketMultiplexers allow multiple sockets to be served by a single
 *  thread.  ESFSocketMultiplexers are threads (ESFThread subclasses) that
 *  manage a list of ESFMultiplexedSockets.  The ESFMultiplexedSocket interface
 *  is in essence a collection of callback functions that will be called by
 *  the ESFMultiplexedSockets whenever read, write, and other events occur.
 *
 *  @ingroup network
 */
class ESFSocketMultiplexer : public ESFThread
{
public:

    /** Construct a new ESFSocketMultiplexer.
     *
     *  @param allocator The allocator that the multiplexer will use to maintain
     *      its list of sockets.  Adding new sockets will cleanly fail when this
     *      allocator is at capacity (e.g., a fixed length allocator with no
     *      failover).
     *  @param logger A logger to log errors to.  Optional.  If NULL, no errors
     *      will be logged.  This logger will be passed to all multiplexed
     *      sockets for error logging and will be used by the multiplexer
     *      itself if necessary.
     */
    ESFSocketMultiplexer( ESFAllocator *allocator, ESFLogger *logger );

    /** Destroy the socket multiplexer.  While this will return all memory
     *  to the allocator supplied in its constructor, it will not notify
     *  any of its sockets that it has been destroyed.
     *  <p>
     *  Important:  The socket multiplexer's thread should not be running when
     *  this destructor function is invoked.
     *  </p>
     *
     *  @see shutdown
     */
    virtual ~ESFSocketMultiplexer();

    /** Add a multiplexed socket to the socket multiplexer.  This can be called
     *  by another thread, but should not be called by a multiplexed socket
     *  within its read, write, or error event handling callbacks.
     *
     *  @param socket The socket to add.
     *  @return true if the socket could be added (if the allocator could
     *      allocate memory), false otherwise.
     */
    bool addSocket( ESFMultiplexedSocket *socket );

    /** Add a multiplexed socket to the socket multiplexer.  This should only
     *  be called by a multiplexed socket within its read, write, or error
     *  handling callbacks.  Unlike addSocket(), this method does not lock
     *  the multiplexer's internal data structures - the multiplexer will
     *  already have these locked when it issues read, write, and error events
     *  to its multiplexed sockets.
     *
     *  @param socket The socket to add.
     *  @return true if the socket could be added (if the allocator could
     *      allocate memory), false otherwise.
     */
    bool addSocketFromCallback( ESFMultiplexedSocket *socket );

    /** Releases all resources held by this multiplexer.  This will decrement
     *  the reference count of all multiplexed sockets and return all memory
     *  to the allocator.  It will also notify all of its sockets that it
     *  no longer manages them - in most cases sockets can simply close
     *  themselves when notified.
     *  <p>
     *  Important: this method should only be called after the multiplexer
     *  has stopped running (i.e., call stop(), then join(), then shutdown() ).
     *  </p>
     *  @see Thread::stop to stop the socket multiplexer's active thread
     *  @see Thread::join to wait for the socket multiplexer's thread to exit.
     */
    void shutdown();

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return The new object or NULL of the memory allocation failed.
     */
    inline void *operator new( size_t size, ESFAllocator *allocator )
    {
        return allocator->allocate( size );
    }

    /** Get the size in bytes of the multiplexer's interal nodes.  This is the
     *  amount of memory that the multiplexer will request from the allocator
     *  for every socket that is added.
     *
     *  @return The size in bytes of the socket's internal nodes.
     */
    static ESFSize GetAllocationSize();

private:

    //  Disabled
    ESFSocketMultiplexer( const ESFSocketMultiplexer &multiplexer );
    ESFSocketMultiplexer &operator=( const ESFSocketMultiplexer &multiplexer );

    void run();
    ESFListIterator removeSocket( ESFListIterator &iterator );

    ESFMutex _mutex;
    ESFList _pendingSockets;
    ESFList _activeSockets;
    ESFLogger *_logger;

    static long _SelectTimeout;                         // milliseconds
    static long _ConnectedSocketTimeout;                // seconds
    static long _ConnectingSocketTimeout;               // seconds
    static long _ListeningSocketTimeout;                // seconds

};

#endif /* ! ESF_SOCKET_MULTIPLEXER_H */
