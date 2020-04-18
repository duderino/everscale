#ifndef ES_HTTP_COMMAND_SOCKET_H
#define ES_HTTP_COMMAND_SOCKET_H

#ifndef ESB_MULTIPLEXED_SOCKET_H
#include <ESBMultiplexedSocket.h>
#endif

#ifndef ESB_EVENT_SOCKET_H
#include <ESBEventSocket.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

namespace ES {

/** A base class for sockets that can wake up multiplexers to run commands.
 */
class HttpCommandSocket : public ESB::MultiplexedSocket {
 public:
  /** Constructor
   */
  HttpCommandSocket();

  /** Destructor.
   */
  virtual ~HttpCommandSocket();

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
   * @return ESB_SUCCESS will keep in multiplexer, ESB_AGAIN will call again,
   * and any other error code will remove socket from multiplexer.
   * Implementations should not close the socket descriptor until handleRemove
   * is called.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual ESB::Error handleAccept(ESB::SocketMultiplexer &multiplexer);

  /**  connected socket has connected to the peer endpoint.
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

 protected:
  /**
   * Enqueue a command on the command socket.  When the command socket is
   * in a multiplexer, the multiplexer will wake up, dequeue the command,
   * and execute it on the multiplexer's thread of control.
   *
   * @param command The command to execute
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error pushInternal(ESB::EmbeddedListElement *command);

  // Subclasses must implement this to downcast EmbeddedListElement to either
  // HttpClientCommand or HttpServerCommand
  virtual ESB::Error runCommand(ESB::EmbeddedListElement *command) = 0;

 private:
  // Disabled
  HttpCommandSocket(const HttpCommandSocket &);
  HttpCommandSocket &operator=(const HttpCommandSocket &);

  ESB::EventSocket _eventSocket;
  ESB::Mutex _lock;
  ESB::EmbeddedList _queue;
  bool _removed;
};

}  // namespace ES

#endif
