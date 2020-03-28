#ifndef ES_HTTP_SERVER_SOCKET_H
#define ES_HTTP_SERVER_SOCKET_H

#ifndef ESB_MULTIPLEXED_SOCKET_H
#include <ESBMultiplexedSocket.h>
#endif

#ifndef ESB_CONNECTED_TCP_SOCKET_H
#include <ESBConnectedTCPSocket.h>
#endif

#ifndef ES_HTTP_SERVER_TRANSACTION_H
#include <ESHttpServerTransaction.h>
#endif

#ifndef ES_HTTP_SERVER_COUNTERS_H
#include <ESHttpServerCounters.h>
#endif

namespace ES {

/** A socket that receives and echoes back HTTP requests
 *
 * TODO implement idle check
 */
class HttpServerSocket : public ESB::MultiplexedSocket {
 public:
  /** Constructor
   *
   * @param cleanupHandler An object that can be used to destroy this one
   */
  HttpServerSocket(HttpServerHandler *handler,
                   ESB::CleanupHandler *cleanupHandler,
                   HttpServerCounters *counters);

  /** Destructor.
   */
  virtual ~HttpServerSocket();

  /** Does this socket want to accept a new connection?  Implies the
   * implementation is a listening socket.
   *
   * @return true if this socket wants to accept a new connection, false
   * otherwise.
   */
  virtual bool wantAccept();

  /** Does this socket want to connect to a peer?  Implies the implementation is
   * a connected socket (client socket, not server socket).
   *
   * @return true if this socket wants to connect to a peer, false otherwise.
   */
  virtual bool wantConnect();

  /** Does this socket want to read data?  Implies the implementation is a
   * connected socket (client socket or server socket)
   *
   * @return true if this socket wants to read data, false otherwise.
   */
  virtual bool wantRead();

  /** Does this socket want to write data?  Implies the implementation is a
   * connected socket (client socket or server socket).
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
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleAccept(ESB::SocketMultiplexer &multiplexer);

  /** Client connected socket has connected to the peer endpoint.
   *
   * @param multiplexer The multiplexer managing this socket.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleConnect(ESB::SocketMultiplexer &multiplexer);

  /** Data is ready to be read.
   *
   * @param multiplexer The multiplexer managing this socket.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleReadable(ESB::SocketMultiplexer &multiplexer);

  /** There is free space in the outgoing socket buffer.
   *
   * @param multiplexer The multiplexer managing this socket.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
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
   * @see handleRemoveEvent to close the socket descriptor.
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
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleRemoteClose(ESB::SocketMultiplexer &multiplexer);

  /** The socket's connection has been idle for too long
   *
   * @param multiplexer The multiplexer managing this socket.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
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
  virtual const char *name() const;

  /** Reset the server socket
   *
   * @param acceptData An object created popupated by ESB::ListeningTCPSockets
   *  when accepting a new connection.
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error reset(HttpServerHandler *handler, ESB::TCPSocket::State &state);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  // Disabled
  HttpServerSocket(const HttpServerSocket &);
  HttpServerSocket &operator=(const HttpServerSocket &);

  ESB::Error parseRequestHeaders(ESB::SocketMultiplexer &multiplexer);
  ESB::Error parseRequestBody(ESB::SocketMultiplexer &multiplexer);
  ESB::Error skipTrailer(ESB::SocketMultiplexer &multiplexer);
  ESB::Error formatResponseHeaders(ESB::SocketMultiplexer &multiplexer);
  ESB::Error formatResponseBody(ESB::SocketMultiplexer &multiplexer);
  ESB::Error flushBuffer(ESB::SocketMultiplexer &multiplexer);
  bool sendResponse(ESB::SocketMultiplexer &multiplexer);
  bool sendBadRequestResponse(ESB::SocketMultiplexer &multiplexer);
  bool sendInternalServerErrorResponse(ESB::SocketMultiplexer &multiplexer);

  int _state;
  int _bodyBytesWritten;
  int _requestsPerConnection;
  ESB::CleanupHandler *_cleanupHandler;
  HttpServerHandler *_handler;
  HttpServerCounters *_counters;
  HttpServerTransaction _transaction;
  ESB::ConnectedTCPSocket _socket;
};

}  // namespace ES

#endif
