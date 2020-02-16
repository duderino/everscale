/** @file ESFConnectedTCPSocket.h
 *  @brief A wrapper class for TCP connected sockets
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 *Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 *both BSD and Apache 2.0 licenses
 *(http://sourceforge.net/projects/sparrowhawk/).
 *
 *	$Author: blattj $
 *	$Date: 2009/05/25 21:51:08 $
 *	$Name:  $
 *	$Revision: 1.3 $
 */

#ifndef ESF_CONNECTED_TCP_SOCKET_H
#define ESF_CONNECTED_TCP_SOCKET_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_SOCKET_H
#include <ESFSocket.h>
#endif

#ifndef ESF_SOCKET_ADDRESS_H
#include <ESFSocketAddress.h>
#endif

#ifndef ESF_ERROR_H
#include <ESFError.h>
#endif

#ifndef ESF_TCP_SOCKET_H
#include <ESFTCPSocket.h>
#endif

#ifndef ESF_BUFFER_H
#include <ESFBuffer.h>
#endif

class ESFListeningTCPSocket;

/** ESFConnectedTCPSockets are used to connect to other TCP endpoints and
 *  send and receive data across the resulting channel.
 *
 *  @ingroup network
 */
class ESFConnectedTCPSocket : public ESFTCPSocket {
 public:
  /** Construct an uninitialized ESFConnectedTCPSocket.
   */
  ESFConnectedTCPSocket();

  /** Construct a new client ESFConnectedTCPSocket.  This instance's
   *  peer will be left uninitialized by this call.
   *
   *  @param isBlocking whether or not this socket is blocking.
   */
  ESFConnectedTCPSocket(bool isBlocking);

  /** Construct a new client ESFConnectedTCPSocket.  This instance will
   *  connect (attempt to connect) to the peer identified by the
   *  ESFSocketAddress instance.
   *
   *  @param peer The peer that this socket will attempt to connect to.
   *  @param isBlocking whether or not this socket is blocking.
   */
  ESFConnectedTCPSocket(const ESFSocketAddress &peer, bool isBlocking);

  /** Construct a new server ESFConnectedTCPSocket.
   *
   * @param acceptData An object populated by ESFListeningTCPSockets
   *  when accepting a new connection.
   */
  ESFConnectedTCPSocket(AcceptData *acceptData);

  /** Destroy the connected socket.  Will close the socket if it has not
   *  already been closed.
   */
  virtual ~ESFConnectedTCPSocket();

  /** Reset a tcp socket.  If the socket is currently open, this will close
   *  it as a side-effect.
   *
   * @param acceptData An object created popupated by ESFListeningTCPSockets
   *  when accepting a new connection.
   * @return ESF_SUCCESS if successful, another error code otherwise.
   */
  virtual ESFError reset(AcceptData *acceptData);

  /** Set the address of the peer.  A client socket will attempt to connect
   *  to this address.
   *
   *  @param address The address of the peer.
   */
  void setPeerAddress(const ESFSocketAddress &address);

  /** Get the address of the peer.
   *
   *  @return address The address of the peer.
   */
  const ESFSocketAddress &getPeerAddress() const;

  /** Get the address of the listening socket that created this connected
   *  socket.
   *
   *  @return The address of the listening socket that created this connected
   *      socket.  If this socket was not created by a listening socket,
   *      the address will be uninitialized (i.e., all fields will be 0).
   */
  const ESFSocketAddress &getListenerAddress() const;

  /** Attempt to connect to the peer.
   *
   *  @return If a blocking socket, returns ESF_SUCCESS if connect succeeded,
   *      another error code otherwise.  If a non-blocking socket, returns
   *      ESF_SUCCESS if connection attempt was successfully initiated,
   *      another error code otherwise.
   */
  ESFError connect();

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
   *  @see ESFTCPSocket::getLastError to get the last error on the socket.
   */
  ESFSSize receive(char *buffer, ESFSize bufferSize);

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
   *  @see ESFTCPSocket::getLastError to get the last error on the socket.
   */
  ESFSSize receive(ESFBuffer *buffer);

  /** Send up to bufferSize bytes from a caller supplied buffer.  This
   *  method returns the number of bytes actually sent.  If a
   *  negative number is returned, than an error has occured and the
   *  caller should call getLastError.
   *
   *  @param buffer The buffer to send.
   *  @param bufferSize The size of the buffer.
   *  @return The number of bytes sent.
   *  @see ESFTCPSocket::getLastError to get the last error on the socket.
   */
  ESFSSize send(const char *buffer, ESFSize bufferSize);

  /** Send up to all of the used space in a buffer.  This
   *  method returns the number of bytes actually sent.  If a
   *  negative number is returned, than an error has occured and the
   *  caller should call getLastError.
   *
   *  @param buffer The buffer to send.
   *  @param bufferSize The size of the buffer.
   *  @return The number of bytes sent.
   *  @see ESFTCPSocket::getLastError to get the last error on the socket.
   */
  ESFSSize send(ESFBuffer *buffer);

  /** Determine whether this is a client or a server socket.  That is,
   *  whether this socket was actively created with the
   *  ESFConnectedTCPSocket::connect() method or whether it was passively
   *  created by the ESFListeningTCPSocket::accept() method.
   *
   *  @return true if this socket is a client socket, false otherwise.
   */
  bool isClient() const;

  /** Get the number of bytes of data that could be read from this socket.
   *
   *  @return The number of bytes that could be read or SOCKET_ERROR if
   *      an error occurred.
   */
  int getBytesReadable();

  /** Get the number of bytes of data that could be read from a socket
   *  descriptor.
   *
   *  @param socketDescriptor The socket descriptor
   *  @return The number of bytes that could be read or SOCKET_ERROR if
   *      an error occurred.
   */
  static int GetBytesReadable(SOCKET socketDescriptor);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  // Disabled
  ESFConnectedTCPSocket(const ESFConnectedTCPSocket &);
  ESFConnectedTCPSocket &operator=(const ESFConnectedTCPSocket &);

  bool _isConnected;
  ESFSocketAddress _listenerAddress;
  ESFSocketAddress _peerAddress;
};

#endif /* ! ESF_CONNECTED_TCP_SOCKET_H */
