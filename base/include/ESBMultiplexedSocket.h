#ifndef ESB_MULTIPLEXED_SOCKET_H
#define ESB_MULTIPLEXED_SOCKET_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_EMBEDDED_MAP_ELEMENT_H
#include <ESBEmbeddedMapElement.h>
#endif

#ifndef ESB_SOCKET_H
#include <ESBSocket.h>
#endif

#ifndef ESB_SOCKET_MULTIPLEXER_H
#include <ESBSocketMultiplexer.h>
#endif

namespace ESB {

/** A socket that can register for and receive events from a socket multiplexer
 *
 *  @ingroup network
 */
class MultiplexedSocket : public EmbeddedMapElement {
 public:
  /** Constructor
   */
  MultiplexedSocket();

  /** Destructor.
   */
  virtual ~MultiplexedSocket();

  /** Does this socket want to accept a new connection?  Implies the
   * implementation is a listening socket.
   *
   * @return true if this socket wants to accept a new connection, false
   * otherwise.
   */
  virtual bool wantAccept() = 0;

  /** Does this socket want to connect to a peer?  Implies the implementation is
   * a connected socket (client socket, not server socket).
   *
   * @return true if this socket wants to connect to a peer, false otherwise.
   */
  virtual bool wantConnect() = 0;

  /** Does this socket want to read data?  Implies the implementation is a
   * connected socket (client socket or server socket)
   *
   * @return true if this socket wants to read data, false otherwise.
   */
  virtual bool wantRead() = 0;

  /** Does this socket want to write data?  Implies the implementation is a
   * connected socket (client socket or server socket).
   *
   * @return true if this socket wants to write data, false otherwise.
   */
  virtual bool wantWrite() = 0;

  /** Is this socket idle?
   *
   * @return true if the socket has been idle for too long, false otherwise.
   */
  virtual bool isIdle() = 0;

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
  virtual Error handleAccept() = 0;

  /** Client connected socket has connected to the peer endpoint.
   *
   * @return ESB_SUCCESS to keep the socket in the multiplexer.  Any other
   * return value will make the multiplexer remove the socket and then call
   * handleRemove().
   * @see handleRemove which should perform necessary cleanup, including
   * possibly closing the socket descriptor.
   */
  virtual Error handleConnect() = 0;

  /** Data is ready to be read.
   *
   * @return ESB_SUCCESS to keep the socket in the multiplexer.  Any other
   * return value will make the multiplexer remove the socket and then call
   * handleRemove().
   * @see handleRemove which should perform necessary cleanup, including
   * possibly closing the socket descriptor.
   */
  virtual Error handleReadable() = 0;

  /** There is free space in the outgoing socket buffer.
   *
   * @return ESB_SUCCESS to keep the socket in the multiplexer.  Any other
   * return value will make the multiplexer remove the socket and then call
   * handleRemove().
   * @see handleRemove which should perform necessary cleanup, including
   * possibly closing the socket descriptor.
   */
  virtual Error handleWritable() = 0;

  /** An error occurred on the socket while waiting for another event.  The
   * error code should be retrieved from the socket itself.
   *
   * @param error The error code.
   * @return true to keep the socket in the multiplexer, false to make the
   * multiplexer remove the socket and then call handleRemove().
   * @see handleRemove which should perform necessary cleanup, including
   * possibly closing the socket descriptor.
   */
  virtual bool handleError(Error error) = 0;

  /** The socket's connection was closed by the remote peer.
   *
   * @return true to keep the socket in the multiplexer, false to make the
   * multiplexer remove the socket and then call handleRemove().
   * @see handleRemove which should perform necessary cleanup, including
   * possibly closing the socket descriptor.
   */
  virtual bool handleRemoteClose() = 0;

  /** The socket's connection has been idle for too long
   *
   * @return true to keep the socket in the multiplexer, false to make the
   * multiplexer remove the socket and then call handleRemove().
   * @see handleRemove which should perform necessary cleanup, including
   * possibly closing the socket descriptor.
   */
  virtual bool handleIdle() = 0;

  /** The socket has been removed from the multiplexer
   *
   * @param multiplexer The multiplexer managing this socket.
   * @return If true, caller should destroy the command with the CleanupHandler.
   */
  virtual bool handleRemove() = 0;

  /** Get the socket's socket descriptor.
   *
   *  @return the socket descriptor
   */
  virtual SOCKET socketDescriptor() const = 0;

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  // Disabled
  MultiplexedSocket(const MultiplexedSocket &);
  MultiplexedSocket &operator=(const MultiplexedSocket &);
};

}  // namespace ESB

#endif
