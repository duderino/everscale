#ifndef ES_HTTP_LISTENING_SOCKET_H
#define ES_HTTP_LISTENING_SOCKET_H

#ifndef ESB_MULTIPLEXED_SOCKET_H
#include <ESBMultiplexedSocket.h>
#endif

#ifndef ESB_LISTENING_TCP_SOCKET_H
#include <ESBListeningTCPSocket.h>
#endif

#ifndef ES_HTTP_SERVER_STACK_H
#include <ESHttpServerStack.h>
#endif

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

namespace ES {

/** A listening socket that creates HttpServerSockets
 */
class HttpListeningSocket : public ESB::MultiplexedSocket {
 public:
  /** Constructor
   */
  HttpListeningSocket(HttpServerStack &stack, HttpServerHandler &handler,
                      ESB::CleanupHandler &cleanupHandler);

  /** Destructor.
   */
  virtual ~HttpListeningSocket();

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
   * @return ESB_SUCCESS will keep in multiplexer, ESB_AGAIN will call again,
   * and any other error code will remove socket from multiplexer.
   * Implementations should not close the socket descriptor until handleRemove
   * is called.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual ESB::Error handleAccept();

  /** Client connected socket has connected to the peer endpoint.
   *
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemove to close the socket descriptor
   */
  virtual bool handleConnect();

  /** Data is ready to be read.
   *
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemove to close the socket descriptor
   */
  virtual bool handleReadable();

  /** There is free space in the outgoing socket buffer.
   *
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemove to close the socket descriptor
   */
  virtual bool handleWritable();

  /** An error occurred on the socket while waiting for another event.  The
   * error code should be retrieved from the socket itself.
   *
   * @param errorCode The error code.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemove to close the socket descriptor.
   * @see TCPSocket::getLastError to get the socket error
   */
  virtual bool handleError(ESB::Error errorCode);

  /** The socket's connection was closed.
   *
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemove to close the socket descriptor
   */
  virtual bool handleRemoteClose();

  /** The socket's connection was closed by this process.
   *
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleLocalClose();

  /** The socket's connection has been idle for too long
   *
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemove to close the socket descriptor
   */
  virtual bool handleIdle();

  /** The socket has been removed from the multiplexer
   *
   * @return If true, caller should destroy the command with the CleanupHandler.
   */
  virtual bool handleRemove();

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

  ESB::Error initialize(ESB::ListeningTCPSocket &socket);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 private:
  // Disabled
  HttpListeningSocket(const HttpListeningSocket &);
  HttpListeningSocket &operator=(const HttpListeningSocket &);

  ESB::ListeningTCPSocket _socket;
  HttpServerStack &_stack;
  HttpServerHandler &_handler;
  ESB::CleanupHandler &_cleanupHandler;
};

}  // namespace ES

#endif /* ! ES_HTTP_LISTENING_SOCKET_H */
