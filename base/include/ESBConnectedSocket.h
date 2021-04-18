#ifndef ESB_CONNECTED_SOCKET_H
#define ESB_CONNECTED_SOCKET_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_SOCKET_TYPE_H
#include <ESBSocketType.h>
#endif

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

#ifndef ESB_SOCKET_H
#include <ESBSocket.h>
#endif

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

#define ESB_SOCK_FLAG_NEW (1 << 0)
#define ESB_SOCK_FLAG_CONNECTING (1 << 1)
#define ESB_SOCK_FLAG_CONNECTED (1 << 2)
#define ESB_SOCK_FLAG_MAX ESB_SOCK_FLAG_CONNECTED
#define ESB_SOCK_FLAG_ALL (ESB_SOCK_FLAG_NEW | ESB_SOCK_FLAG_CONNECTING | ESB_SOCK_FLAG_CONNECTED)

namespace ESB {

class ListeningSocket;

/** ConnectedSocket is an abstract base class for plain and encrypted connected sockets.
 *
 *  @ingroup network
 */
class ConnectedSocket : public Socket {
 public:
  /** Construct a new server ConnectedSocket.
   *
   * @param acceptState init parameters created by the ListeningSocket
   * @param namePrefix A name prefix to be incorporated into log messages
   */
  ConnectedSocket(const Socket::State &acceptState, const char *namePrefix);

  /** Construct a new client ConnectedSocket.
   *
   * @param namePrefix A name prefix to be incorporated into log messages
   * @param isBlocking whether or not this socket is blocking.
   */
  ConnectedSocket(const char *namePrefix, bool isBlocking = false);

  /** Destroy the connected socket.  Will close the socket if it has not
   *  already been closed.
   */
  virtual ~ConnectedSocket();

  /* Get the socket peer's ipaddr+port in human-friendly presentation format.
   *
   * @return The peer's presentation address.
   */
  virtual const char *name() const;

  /** Get the address of the peer.
   *
   *  @return address The address of the peer.
   */
  virtual const SocketAddress &peerAddress() const = 0;

  /** Get the address of the listening socket that created this connected
   *  socket.
   *
   *  @return The address of the listening socket that created this connected
   *      socket.  If this socket was not created by a listening socket,
   *      the address will be uninitialized (i.e., all fields will be 0).
   */
  const SocketAddress &localAddress() const;

  /** Attempt to connect to the peer.
   *
   *  @return If a blocking socket, returns ESB_SUCCESS if connect succeeded,
   *      another error code otherwise.  If a non-blocking socket, returns
   *      ESB_SUCCESS if connection attempt was successfully initiated,
   *      another error code otherwise.
   */
  Error connect();

  /** Close the socket.
   */
  virtual void close();

  /**
   * Determine whether the socket uses encryption
   *
   * @return true if the socket uses encryption
   */
  virtual bool secure() const = 0;

  /** Determine whether there is a communications channel currently open
   *  between this socket and the peer.  This is useful for knowing when a
   *  non-blocking connect has succeeded.
   *
   *  @return true if the socket is connected, false otherwise.
   */
  bool connected();

  /** Read up to bufferSize bytes into a caller supplied buffer.  This
   *  method returns the number of bytes actually read.  If this is a
   *  blocking socket and zero bytes were read, then the peer has closed
   *  the socket connection.  If this is a non-blocking socket and zero
   *  bytes were read, then there was simply nothing to read.  If a
   *  negative number is returned, than a true error has occured and the
   *  caller should call getLastError.
   *
   *  @param buffer The buffer to read into.
   *  @param bufferSize The size of the buffer.
   *  @return The number of bytes read
   *  @see TCPSocket::getLastError to get the last error on the socket.
   */
  virtual SSize receive(char *buffer, Size bufferSize);

  /** Fill up to al of the free space in a buffer.  This
   *  method returns the number of bytes actually read.  If this is a
   *  blocking socket and zero bytes were read, then the peer has closed
   *  the socket connection.  If this is a non-blocking socket and zero
   *  bytes were read, then there was simply nothing to read.  If a
   *  negative number is returned, than a true error has occured and the
   *  caller should call getLastError.
   *
   *  @param buffer The buffer to read into.
   *  @return The number of bytes read
   *  @see TCPSocket::getLastError to get the last error on the socket.
   */
  SSize receive(Buffer *buffer);

  /** Send up to bufferSize bytes from a caller supplied buffer.  This
   *  method returns the number of bytes actually sent.  If a
   *  negative number is returned, than an error has occured and the
   *  caller should call getLastError.
   *
   *  @param buffer The buffer to send.
   *  @param bufferSize The size of the buffer.
   *  @return The number of bytes sent.
   *  @see TCPSocket::getLastError to get the last error on the socket.
   */
  virtual SSize send(const char *buffer, Size bufferSize);

  /** Send up to all of the used space in a buffer.  This
   *  method returns the number of bytes actually sent.  If a
   *  negative number is returned, than an error has occured and the
   *  caller should call getLastError.
   *
   *  @param buffer The buffer to send.
   *  @param bufferSize The size of the buffer.
   *  @return The number of bytes sent.
   *  @see TCPSocket::getLastError to get the last error on the socket.
   */
  SSize send(Buffer *buffer);

  /**
   * Determine whether the socket impl needs to receive more data to make progress (e.g., to complete a TLS handshake)
   *
   * @return true if the socket impl needs to receive more data to make progress
   */
  virtual bool wantRead();

  /**
   * Determine whether the socket impl needs to send more data to make progress (e.g,. to complete a TLS handshake)
   *
   * @return true if the socket impl needs to send more data to make progress
   */
  virtual bool wantWrite();

  /** Get the number of bytes of data that could be read from this socket.
   *
   *  @return The number of bytes that could be read or SOCKET_ERROR if
   *      an error occurred.
   */
  int bytesReadable();

  /** Get the number of bytes of data that could be read from a socket
   *  descriptor.
   *
   *  @param socketDescriptor The socket descriptor
   *  @return The number of bytes that could be read or SOCKET_ERROR if
   *      an error occurred.
   */
  static int BytesReadable(SOCKET socketDescriptor);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param memory The object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::EmbeddedListElement *memory) noexcept { return memory; }

 protected:
  int _flags;

 private:
  // Disabled
  ConnectedSocket(const ConnectedSocket &);
  ConnectedSocket &operator=(const ConnectedSocket &);

  void formatPrefix(const char *namePrefix, const char *nameSuffix);
  void updateName(const SocketAddress &peerAddress);
  ESB::Error updateLocalAddress();

  SocketAddress _localAddress;

  // <prefix>:<ip addr>:<port>-<ip addr>:<port>,<port>
  mutable char _logAddress[ESB_NAME_PREFIX_SIZE + 1 + ESB_ADDRESS_PORT_SIZE + 1 + ESB_ADDRESS_PORT_SIZE + 1 +
                           ESB_MAX_UINT32_STRING_LENGTH];
};

}  // namespace ESB

#endif
