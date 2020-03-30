#ifndef ES_HTTP_CLIENT_SOCKET_H
#define ES_HTTP_CLIENT_SOCKET_H

#ifndef ESB_MULTIPLEXED_SOCKET_H
#include <ESBMultiplexedSocket.h>
#endif

#ifndef ESB_CONNECTED_TCP_SOCKET_H
#include <ESBConnectedTCPSocket.h>
#endif

#ifndef ES_HTTP_CLIENT_COUNTERS_H
#include <ESHttpClientCounters.h>
#endif

#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

#ifndef ES_HTTP_CONNECTION_POOL_H
#include <ESHttpConnectionPool.h>
#endif

namespace ES {

/** A socket that receives and echoes back HTTP requests
 *
 * TODO implement idle check
 */
class HttpClientSocket : public ESB::MultiplexedSocket {
 public:
  /**
   * If a HttpClientSocket fails before sending the transaction, it can use
   * this to retry the transaction on a different socket.
   */
  class RetryHandler {
   public:
    virtual ESB::Error retry(HttpClientTransaction *transaction) = 0;
  };

  /** Constructor
   *
   * @param transaction The client transaction object.  Many client transactions
   *  can be carried across the same http client socket with connection reuse.
   * @param cleanupHandler An object that can be used to destroy this one
   */
  HttpClientSocket(RetryHandler &retryHandler,
                   HttpClientTransaction *transaction,
                   HttpClientCounters *counters,
                   ESB::CleanupHandler *cleanupHandler);

  /** Destructor.
   */
  virtual ~HttpClientSocket();

  /** Does this socket want to accept a new connection?  Implies the
   * implementation is a listening socket.
   *
   * @return true if this socket wants to accept a new connection, false
   * otherwise.
   */
  virtual bool wantAccept();

  /** Does this socket want to connect to a peer?  Implies the implementation is
   * a connected socket (client socket, not client socket).
   *
   * @return true if this socket wants to connect to a peer, false otherwise.
   */
  virtual bool wantConnect();

  /** Does this socket want to read data?  Implies the implementation is a
   * connected socket (client socket or client socket)
   *
   * @return true if this socket wants to read data, false otherwise.
   */
  virtual bool wantRead();

  /** Does this socket want to write data?  Implies the implementation is a
   * connected socket (client socket or client socket).
   *
   * @return true if this socket wants to write data, false otherwise.
   */
  virtual bool wantWrite();

  /** Is this socket idle?
   *
   * @return true if the socket has been idle for too long, false otherwise.
   */
  virtual bool isIdle();

  /** Listening socket may be ready to accept a new connection.  If multiple
   * threads are waiting on the same non-blocking listening socket, one or more
   * threads may not actually be able to accept a new connection when this is
   * called.  This is not an error condition.
   *
   * @param multiplexer The multiplexer managing this socket.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemove to close the socket descriptor
   */
  virtual bool handleAccept(ESB::SocketMultiplexer &multiplexer);

  /** Client connected socket has connected to the peer endpoint.
   *
   * @param multiplexer The multiplexer managing this socket.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemove to close the socket descriptor
   */
  virtual bool handleConnect(ESB::SocketMultiplexer &multiplexer);

  /** Data is ready to be read.
   *
   * @param multiplexer The multiplexer managing this socket.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemove to close the socket descriptor
   */
  virtual bool handleReadable(ESB::SocketMultiplexer &multiplexer);

  /** There is free space in the outgoing socket buffer.
   *
   * @param multiplexer The multiplexer managing this socket.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemove to close the socket descriptor
   */
  virtual bool handleWritable(ESB::SocketMultiplexer &multiplexer);

  /** An error occurred on the socket while waiting for another event.  The
   * error code should be retrieved from the socket itself.
   *
   * @param errorCode The error code.
   * @param multiplexer The multiplexer managing this socket.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemove to close the socket descriptor.
   * @see TCPSocket::getLastError to get the socket error
   */
  virtual bool handleError(ESB::Error errorCode,
                           ESB::SocketMultiplexer &multiplexer);

  /** The socket's connection was closed.
   *
   * @param multiplexer The multiplexer managing this socket.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemove to close the socket descriptor
   */
  virtual bool handleRemoteClose(ESB::SocketMultiplexer &multiplexer);

  /** The socket's connection has been idle for too long
   *
   * @param multiplexer The multiplexer managing this socket.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemove to close the socket descriptor
   */
  virtual bool handleIdle(ESB::SocketMultiplexer &multiplexer);

  /** The socket has been removed from the multiplexer
   *
   * @param multiplexer The multiplexer managing this socket.
   * @return If true, caller should destroy the command with the CleanupHandler.
   */
  virtual bool handleRemove(ESB::SocketMultiplexer &multiplexer);

  /** Get the socket's socket descriptor.
   *
   *  @return the socket descriptor
   */
  virtual SOCKET socketDescriptor() const;

  /** Return an optional handler that can destroy the multiplexer.
   *
   * @return A handler to destroy the element or NULL if the element should not
   * be destroyed.
   */
  virtual ESB::CleanupHandler *cleanupHandler();

  /** Get the name of the multiplexer.  This name can be used in logging
   * messages, etc.
   *
   * @return The multiplexer's name
   */
  virtual const char *getName() const;

  /** Reset the client socket
   *
   * @param reused true if the socket is being reused for a new transaction
   * @param transaction The client transaction object.  Many client transactions
   *  can be carried across the same http client socket with connection reuse.
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error reset(bool reused, HttpClientTransaction *transaction);

  inline void close() { _socket.close(); }

  inline ESB::Error connect() { return _socket.connect(); }

  inline bool isConnected() { return _socket.isConnected(); }

  inline const ESB::SocketAddress *peerAddress() const {
    return &_socket.peerAddress();
  }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

  static inline void SetReuseConnections(bool reuseConnections) {
    _ReuseConnections = reuseConnections;
  }

  static inline bool GetReuseConnections() { return _ReuseConnections; }

 private:
  // Disabled
  HttpClientSocket(const HttpClientSocket &);
  HttpClientSocket &operator=(const HttpClientSocket &);

  ESB::Error parseResponseHeaders(ESB::SocketMultiplexer &multiplexer);
  ESB::Error parseResponseBody(ESB::SocketMultiplexer &multiplexer);
  ESB::Error formatRequestHeaders(ESB::SocketMultiplexer &multiplexer);
  ESB::Error formatRequestBody(ESB::SocketMultiplexer &multiplexer);
  ESB::Error flushBuffer(ESB::SocketMultiplexer &multiplexer);

  int _state;
  int _bodyBytesWritten;
  RetryHandler &_retryHandler;
  HttpClientTransaction *_transaction;
  HttpClientCounters *_counters;
  ESB::CleanupHandler *_cleanupHandler;
  ESB::ConnectedTCPSocket _socket;
  static bool _ReuseConnections;
};

}  // namespace ES

#endif
