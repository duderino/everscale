#ifndef EST_LISTENING_SOCKET_H
#define EST_LISTENING_SOCKET_H

#ifndef ESB_MULTIPLEXED_SOCKET_H
#include <ESBMultiplexedSocket.h>
#endif

#ifndef ESB_LISTENING_TCP_SOCKET_H
#include <ESBListeningTCPSocket.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_SOCKET_MULTIPLEXER_DISPATCHER_H
#include <ESBSocketMultiplexerDispatcher.h>
#endif

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

namespace EST {

/** A listening socket that creates ESTServerSockets
 */
class ListeningSocket : public ESB::MultiplexedSocket {
 public:
  /** Constructor
   *
   * @param socket A fully initialized (after bind() and listen()) listening
   * socket to accept new requests from.
   * @param allocator An allocator that will allocate new ESTServerSockets
   * @param dispatcher A dispatcher to which newly accepted connections will be
   * added.
   * @param thisCleanupHandler A cleanup handler for this object
   * @param socketCleanupHandler A cleanup handler for all ESTServerSockets
   * created by this object.
   */
  ListeningSocket(ESB::ListeningTCPSocket *socket, ESB::Allocator *allocator,
                  ESB::SocketMultiplexerDispatcher *dispatcher,
                  ESB::CleanupHandler *thisCleanupHandler,
                  ESB::CleanupHandler *socketCleanupHandler);

  /** Destructor.
   */
  virtual ~ListeningSocket();

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
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleAcceptEvent(ESB::SharedInt *isRunning);

  /** Client connected socket has connected to the peer endpoint.
   *
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleConnectEvent(ESB::SharedInt *isRunning);

  /** Data is ready to be read.
   *
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleReadableEvent(ESB::SharedInt *isRunning);

  /** There is free space in the outgoing socket buffer.
   *
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleWritableEvent(ESB::SharedInt *isRunning);

  /** An error occurred on the socket while waiting for another event.  The
   * error code should be retrieved from the socket itself.
   *
   * @param errorCode The error code.
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor.
   * @see ESBTCPSocket::getLastError to get the socket error
   */
  virtual bool handleErrorEvent(ESB::Error errorCode,
                                ESB::SharedInt *isRunning);

  /** The socket's connection was closed.
   *
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleEndOfFileEvent(ESB::SharedInt *isRunning);

  /** The socket's connection has been idle for too long
   *
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleIdleEvent(ESB::SharedInt *isRunning);

  /** The socket has been removed from the multiplexer
   *
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @return If true, caller should destroy the command with the CleanupHandler.
   */
  virtual bool handleRemoveEvent(ESB::SharedInt *isRunning);

  /** Get the socket's socket descriptor.
   *
   *  @return the socket descriptor
   */
  virtual SOCKET getSocketDescriptor() const;

  /** Return an optional handler that can destroy the multiplexer.
   *
   * @return A handler to destroy the element or NULL if the element should not
   * be destroyed.
   */
  virtual ESB::CleanupHandler *getCleanupHandler();

  /** Get the name of the multiplexer.  This name can be used in logging
   * messages, etc.
   *
   * @return The multiplexer's name
   */
  virtual const char *getName() const;

  /** Run the command
   *
   * @param isRunning This object will return true as long as the controlling
   * thread isRunning, false when the controlling thread wants to shutdown.
   * @return If true, caller should destroy the command with the CleanupHandler.
   */
  virtual bool run(ESB::SharedInt *isRunning);

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
  ListeningSocket(const ListeningSocket &);
  ListeningSocket &operator=(const ListeningSocket &);

  ESB::ListeningTCPSocket *_socket;
  ESB::Allocator *_allocator;
  ESB::SocketMultiplexerDispatcher *_dispatcher;
  ESB::CleanupHandler *_thisCleanupHandler;
  ESB::CleanupHandler *_socketCleanupHandler;
};

}  // namespace EST

#endif
