#ifndef ESB_CONNECTED_TCP_SOCKET_H
#define ESB_CONNECTED_TCP_SOCKET_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_SOCKET_H
#include <ESBSocket.h>
#endif

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

#ifndef ESB_TCP_SOCKET_H
#include <ESBTCPSocket.h>
#endif

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

namespace ESB {

class ListeningTCPSocket;

/** ConnectedTCPSockets are used to connect to other TCP endpoints and
 *  send and receive data across the resulting channel.
 *
 *  @ingroup network
 */
class ConnectedTCPSocket : public TCPSocket {
 public:
  /** Construct an uninitialized ConnectedTCPSocket.
   */
  ConnectedTCPSocket();

  /** Construct a new client ConnectedTCPSocket.  This instance's
   *  peer will be left uninitialized by this call.
   *
   *  @param isBlocking whether or not this socket is blocking.
   */
  ConnectedTCPSocket(bool isBlocking);

  /** Construct a new client ConnectedTCPSocket.  This instance will
   *  connect (attempt to connect) to the peer identified by the
   *  SocketAddress instance.
   *
   *  @param peer The peer that this socket will attempt to connect to.
   *  @param isBlocking whether or not this socket is blocking.
   */
  ConnectedTCPSocket(const SocketAddress &peer, bool isBlocking);

  /** Construct a new server ConnectedTCPSocket.
   *
   * @param state An object populated by ListeningTCPSockets
   *  when accepting a new connection.
   */
  ConnectedTCPSocket(const State &state);

  /** Destroy the connected socket.  Will close the socket if it has not
   *  already been closed.
   */
  virtual ~ConnectedTCPSocket();

  /** Reset a tcp socket.  If the socket is currently open, this will close
   *  it as a side-effect.
   *
   * @param acceptData An object created popupated by ListeningTCPSockets
   *  when accepting a new connection.
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error reset(const State &acceptData);

  /** Set the address of the peer.  A client socket will attempt to connect
   *  to this address.
   *
   *  @param address The address of the peer.
   */
  void setPeerAddress(const SocketAddress &address);

  /** Get the address of the peer.
   *
   *  @return address The address of the peer.
   */
  const SocketAddress &peerAddress() const;

  /** Get the address of the listening socket that created this connected
   *  socket.
   *
   *  @return The address of the listening socket that created this connected
   *      socket.  If this socket was not created by a listening socket,
   *      the address will be uninitialized (i.e., all fields will be 0).
   */
  const SocketAddress &listenerAddress() const;

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

  /** Determine whether there is a communications channel currently open
   *  between this socket and the peer.  This is useful for knowing when a
   *  non-blocking connect has succeeded.
   *
   *  @return true if the socket is connected, false otherwise.
   */
  bool isConnected();

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
  SSize receive(char *buffer, Size bufferSize);

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
  SSize send(const char *buffer, Size bufferSize);

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

  /** Determine whether this is a client or a server socket.  That is,
   *  whether this socket was actively created with the
   *  ConnectedTCPSocket::connect() method or whether it was passively
   *  created by the ListeningTCPSocket::accept() method.
   *
   *  @return true if this socket is a client socket, false otherwise.
   */
  bool isClient() const;

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
  inline void *operator new(size_t size, Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 private:
  // Disabled
  ConnectedTCPSocket(const ConnectedTCPSocket &);
  ConnectedTCPSocket &operator=(const ConnectedTCPSocket &);

  bool _isConnected;
  SocketAddress _listenerAddress;
  SocketAddress _peerAddress;
};

}  // namespace ESB

#endif
