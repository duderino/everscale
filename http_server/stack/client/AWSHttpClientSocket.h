/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_CLIENT_SOCKET_H
#define AWS_HTTP_CLIENT_SOCKET_H

#ifndef ESF_MULTIPLEXED_SOCKET_H
#include <ESFMultiplexedSocket.h>
#endif

#ifndef ESF_CONNECTED_TCP_SOCKET_H
#include <ESFConnectedTCPSocket.h>
#endif

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

#ifndef AWS_PERFORMANCE_COUNTER_H
#include <AWSPerformanceCounter.h>
#endif

#ifndef AWS_HTTP_CLIENT_TRANSACTION_H
#include <AWSHttpClientTransaction.h>
#endif

#ifndef AWS_HTTP_CONNECTION_POOL_H
#include <AWSHttpConnectionPool.h>
#endif


/** A socket that receives and echoes back HTTP requests
 *
 * TODO implement idle check
 */
class AWSHttpClientSocket : public ESFMultiplexedSocket
{
public:

    /** Constructor
     *
     * @param transaction The client transaction object.  Many client transactions
     *  can be carried across the same http client socket with connection reuse.
     * @param cleanupHandler An object that can be used to destroy this one
     * @param logger A logger
     */
    AWSHttpClientSocket(AWSHttpConnectionPool *pool,
                        AWSHttpClientTransaction *transaction,
                        AWSPerformanceCounter *successCounter,
                        AWSPerformanceCounter *failureCounter,
                        ESFCleanupHandler *cleanupHandler,
                        ESFLogger *logger);

    /** Destructor.
     */
    virtual ~AWSHttpClientSocket();

    /** Does this socket want to accept a new connection?  Implies the implementation
     *  is a listening socket.
     *
     * @return true if this socket wants to accept a new connection, false otherwise.
     */
    virtual bool wantAccept();

    /** Does this socket want to connect to a peer?  Implies the implementation is a
     *  connected socket (client socket, not client socket).
     *
     * @return true if this socket wants to connect to a peer, false otherwise.
     */
    virtual bool wantConnect();

    /** Does this socket want to read data?  Implies the implementation is a connected
     *  socket (client socket or client socket)
     *
     * @return true if this socket wants to read data, false otherwise.
     */
    virtual bool wantRead();

    /** Does this socket want to write data?  Implies the implementation is a connected
     *  socket (client socket or client socket).
     *
     * @return true if this socket wants to write data, false otherwise.
     */
    virtual bool wantWrite();

    /** Is this socket idle?
     *
     * @return true if the socket has been idle for too long, false otherwise.
     */
    virtual bool isIdle();

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
    virtual bool handleAcceptEvent(ESFFlag *isRunning, ESFLogger *logger);

    /** Client connected socket has connected to the peer endpoint.
     *
     * @param isRunning If this object returns false, this method should return as soon
     *  as possible.
     * @param logger Log messages should be sent to this object.
     * @return If true keep in the multiplexer, if false remove from the multiplexer.
     *  Do not close the socket descriptor until after the socket has been removed.
     * @see handleRemoveEvent to close the socket descriptor
     */
    virtual bool handleConnectEvent(ESFFlag *isRunning, ESFLogger *logger);

    /** Data is ready to be read.
     *
     * @param isRunning If this object returns false, this method should return as soon
     *  as possible.
     * @param logger Log messages should be sent to this object.
     * @return If true keep in the multiplexer, if false remove from the multiplexer.
     *  Do not close the socket descriptor until after the socket has been removed.
     * @see handleRemoveEvent to close the socket descriptor
     */
    virtual bool handleReadableEvent(ESFFlag *isRunning, ESFLogger *logger);

    /** There is free space in the outgoing socket buffer.
     *
     * @param isRunning If this object returns false, this method should return as soon
     *  as possible.
     * @param logger Log messages should be sent to this object.
     * @return If true keep in the multiplexer, if false remove from the multiplexer.
     *  Do not close the socket descriptor until after the socket has been removed.
     * @see handleRemoveEvent to close the socket descriptor
     */
    virtual bool handleWritableEvent(ESFFlag *isRunning, ESFLogger *logger);

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
    virtual bool handleErrorEvent(ESFError errorCode, ESFFlag *isRunning, ESFLogger *logger);

    /** The socket's connection was closed.
     *
     * @param isRunning If this object returns false, this method should return as soon
     *  as possible.
     * @param logger Log messages should be sent to this object.
     * @return If true keep in the multiplexer, if false remove from the multiplexer.
     *  Do not close the socket descriptor until after the socket has been removed.
     * @see handleRemoveEvent to close the socket descriptor
     */
    virtual bool handleEndOfFileEvent(ESFFlag *isRunning, ESFLogger *logger);

    /** The socket's connection has been idle for too long
     *
     * @param isRunning If this object returns false, this method should return as soon
     *  as possible.
     * @param logger Log messages should be sent to this object.
     * @return If true keep in the multiplexer, if false remove from the multiplexer.
     *  Do not close the socket descriptor until after the socket has been removed.
     * @see handleRemoveEvent to close the socket descriptor
     */
    virtual bool handleIdleEvent(ESFFlag *isRunning, ESFLogger *logger);

    /** The socket has been removed from the multiplexer
     *
     * @param isRunning If this object returns false, this method should return as soon
     *  as possible.
     * @param logger Log messages should be sent to this object.
     * @return If true, caller should destroy the command with the CleanupHandler.
     */
    virtual bool handleRemoveEvent(ESFFlag *isRunning, ESFLogger *logger);

    /** Get the socket's socket descriptor.
     *
     *  @return the socket descriptor
     */
    virtual SOCKET getSocketDescriptor() const;

    /** Return an optional handler that can destroy the multiplexer.
     *
     * @return A handler to destroy the element or NULL if the element should not be destroyed.
     */
    virtual ESFCleanupHandler *getCleanupHandler();

    /** Get the name of the multiplexer.  This name can be used in logging messages, etc.
     *
     * @return The multiplexer's name
     */
    virtual const char *getName() const;

    /** Run the command
     *
     * @param isRunning This object will return true as long as the controlling thread
     *  isRunning, false when the controlling thread wants to shutdown.
     * @return If true, caller should destroy the command with the CleanupHandler.
     */
    virtual bool run(ESFFlag *isRunning);

    /** Reset the client socket
     *
     * @param reused true if the socket is being reused for a new transaction
     * @param transaction The client transaction object.  Many client transactions
     *  can be carried across the same http client socket with connection reuse.
     * @return ESF_SUCCESS if successful, another error code otherwise.
     */
    ESFError reset(bool reused, AWSHttpConnectionPool *pool, AWSHttpClientTransaction *transaction);

    inline void close()
    {
        _socket.close();
    }

    inline ESFError connect()
    {
        return _socket.connect();
    }

    inline bool isConnected()
    {
        return _socket.isConnected();
    }

    inline const ESFSocketAddress *getPeerAddress() const
    {
        return &_socket.getPeerAddress();
    }

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return Memory for the new object or NULL if the memory allocation failed.
     */
    inline void *operator new(size_t size, ESFAllocator *allocator)
    {
        return allocator->allocate(size);
    }

    static inline void SetReuseConnections(bool reuseConnections)
    {
        _ReuseConnections = reuseConnections;
    }

    static inline bool GetReuseConnections()
    {
        return _ReuseConnections;
    }

private:
    // Disabled
    AWSHttpClientSocket(const AWSHttpClientSocket &);
    AWSHttpClientSocket &operator=(const AWSHttpClientSocket &);

    ESFError parseResponseHeaders(ESFFlag *isRunning, ESFLogger *logger);

    ESFError parseResponseBody(ESFFlag *isRunning, ESFLogger *logger);

    ESFError formatRequestHeaders(ESFFlag *isRunning, ESFLogger *logger);

    ESFError formatRequestBody(ESFFlag *isRunning, ESFLogger *logger);

    ESFError flushBuffer(ESFFlag *isRunning, ESFLogger *logger);

    int _state;
    int _bodyBytesWritten;
    AWSHttpConnectionPool *_pool;
    AWSHttpClientTransaction *_transaction;
    AWSPerformanceCounter *_successCounter;
    AWSPerformanceCounter *_failureCounter;
    ESFLogger *_logger;
    ESFCleanupHandler *_cleanupHandler;
    ESFConnectedTCPSocket _socket;

    static bool _ReuseConnections;
};

#endif

