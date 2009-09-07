/** @file ESFMultiplexedSocket.h
 *  @brief A socket that can register for and receive events from a socket multiplexer
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_MULTIPLEXED_SOCKET_H
#define ESF_MULTIPLEXED_SOCKET_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_COMMAND_H
#include <ESFCommand.h>
#endif

#ifndef ESF_SOCKET_H
#include <ESFSocket.h>
#endif

#ifndef ESF_FLAG_H
#include <ESFFlag.h>
#endif

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

/** A socket that can register for and receive events from a socket multiplexer
 *
 *  @ingroup network
 */
class ESFMultiplexedSocket: public ESFCommand {
public:

    /** Constructor
     */
    ESFMultiplexedSocket();

    /** Destructor.
     */
    virtual ~ESFMultiplexedSocket();

    /** Does this socket want to accept a new connection?  Implies the implementation
     *  is a listening socket.
     *
     * @return true if this socket wants to accept a new connection, false otherwise.
     */
    virtual bool wantAccept() = 0;

    /** Does this socket want to connect to a peer?  Implies the implementation is a
     *  connected socket (client socket, not server socket).
     *
     * @return true if this socket wants to connect to a peer, false otherwise.
     */
    virtual bool wantConnect() = 0;

    /** Does this socket want to read data?  Implies the implementation is a connected
     *  socket (client socket or server socket)
     *
     * @return true if this socket wants to read data, false otherwise.
     */
    virtual bool wantRead() = 0;

    /** Does this socket want to write data?  Implies the implementation is a connected
     *  socket (client socket or server socket).
     *
     * @return true if this socket wants to write data, false otherwise.
     */
    virtual bool wantWrite() = 0;

    /** Is this socket idle?
     *
     * @return true if the socket has been idle for too long, false otherwise.
     */
    virtual bool isIdle() = 0;

    /** Listening socket may be ready to accept a new connection.  If multiple threads
     *  are waiting on the same non-blocking listening socket, one or more threads may not
     *  actually be able to accept a new connection when this is called.  This is not
     *  an error condition.
     *
     * @param isRunning If this object returns false, this method should return as soon
     *  as possible.
     * @param logger Log messages should be sent to this object.
     * @return If true keep in the multiplexer, if false remove from the multiplexer.
     *  Do not close the socket descriptor until after the socket has been removed.
     * @see handleRemoveEvent to close the socket descriptor
     */
    virtual bool handleAcceptEvent(ESFFlag *isRunning, ESFLogger *logger) = 0;

    /** Client connected socket has connected to the peer endpoint.
     *
     * @param isRunning If this object returns false, this method should return as soon
     *  as possible.
     * @param logger Log messages should be sent to this object.
     * @return If true keep in the multiplexer, if false remove from the multiplexer.
     *  Do not close the socket descriptor until after the socket has been removed.
     * @see handleRemoveEvent to close the socket descriptor
     */
    virtual bool handleConnectEvent(ESFFlag *isRunning, ESFLogger *logger) = 0;

    /** Data is ready to be read.
     *
     * @param isRunning If this object returns false, this method should return as soon
     *  as possible.
     * @param logger Log messages should be sent to this object.
     * @return If true keep in the multiplexer, if false remove from the multiplexer.
     *  Do not close the socket descriptor until after the socket has been removed.
     * @see handleRemoveEvent to close the socket descriptor
     */
    virtual bool handleReadableEvent(ESFFlag *isRunning, ESFLogger *logger) = 0;

    /** There is free space in the outgoing socket buffer.
     *
     * @param isRunning If this object returns false, this method should return as soon
     *  as possible.
     * @param logger Log messages should be sent to this object.
     * @return If true keep in the multiplexer, if false remove from the multiplexer.
     *  Do not close the socket descriptor until after the socket has been removed.
     * @see handleRemoveEvent to close the socket descriptor
     */
    virtual bool handleWritableEvent(ESFFlag *isRunning, ESFLogger *logger) = 0;

    /** An error occurred on the socket while waiting for another event.  The error code
     *  should be retrieved from the socket itself.
     *
     * @param errorCode The error code.
     * @param isRunning If this object returns false, this method should return as soon
     *  as possible.
     * @param logger Log messages should be sent to this object.
     * @return If true keep in the multiplexer, if false remove from the multiplexer.
     *  Do not close the socket descriptor until after the socket has been removed.
     * @see handleRemoveEvent to close the socket descriptor.
     * @see ESFTCPSocket::getLastError to get the socket error
     */
    virtual bool handleErrorEvent(ESFError errorCode, ESFFlag *isRunning, ESFLogger *logger) = 0;

    /** The socket's connection was closed.
     *
     * @param isRunning If this object returns false, this method should return as soon
     *  as possible.
     * @param logger Log messages should be sent to this object.
     * @return If true keep in the multiplexer, if false remove from the multiplexer.
     *  Do not close the socket descriptor until after the socket has been removed.
     * @see handleRemoveEvent to close the socket descriptor
     */
    virtual bool handleEndOfFileEvent(ESFFlag *isRunning, ESFLogger *logger) = 0;

    /** The socket's connection has been idle for too long
     *
     * @param isRunning If this object returns false, this method should return as soon
     *  as possible.
     * @param logger Log messages should be sent to this object.
     * @return If true keep in the multiplexer, if false remove from the multiplexer.
     *  Do not close the socket descriptor until after the socket has been removed.
     * @see handleRemoveEvent to close the socket descriptor
     */
    virtual bool handleIdleEvent(ESFFlag *isRunning, ESFLogger *logger) = 0;

    /** The socket has been removed from the multiplexer
     *
     * @param isRunning If this object returns false, this method should return as soon
     *  as possible.
     * @param logger Log messages should be sent to this object.
     * @return If true, caller should destroy the command with the CleanupHandler.
     */
    virtual bool handleRemoveEvent(ESFFlag *isRunning, ESFLogger *logger) = 0;

    /** Get the socket's socket descriptor.
     *
     *  @return the socket descriptor
     */
    virtual SOCKET getSocketDescriptor() const = 0;

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return Memory for the new object or NULL if the memory allocation failed.
     */
    inline void *operator new(size_t size, ESFAllocator *allocator) {
        return allocator->allocate(size);
    }

private:
    // Disabled
    ESFMultiplexedSocket(const ESFMultiplexedSocket &);
    ESFMultiplexedSocket &operator=(const ESFMultiplexedSocket &);
};

#endif /* ! ESF_MULTIPLEXED_SOCKET_H */
