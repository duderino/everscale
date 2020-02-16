/** @file ESFListeningTCPSocket.h
 *  @brief A TCP socket bound to a port and ip address that is capable of
 *      receiving incoming connections.
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

#ifndef ESF_LISTENING_TCP_SOCKET_H
#define ESF_LISTENING_TCP_SOCKET_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_SOCKET_H
#include <ESFSocket.h>
#endif

#ifndef ESF_SOCKET_ADDRESS_H
#include <ESFSocketAddress.h>
#endif

#ifndef ESF_TCP_SOCKET_H
#include <ESFTCPSocket.h>
#endif

#ifndef ESF_CONNECTED_TCP_SOCKET_H
#include <ESFConnectedTCPSocket.h>
#endif

#ifndef ESF_ERROR_H
#include <ESFError.h>
#endif

/** @todo allow binding to a ESFSocketAddress - bind() with no args will
 *  bind to INADDR_ANY
 */

/** ESFListeningTCPSockets are used to receive connections from other TCP
 *  endpoints.  New connections are wrapped in ESFConnectedTCPSockets.
 *
 *  @ingroup network
 */
class ESFListeningTCPSocket : public ESFTCPSocket {
 public:
  /** Construct a new ESFListeningTCPSocket instance.  This socket will
   *  listen on the specified port and let the kernel choose the IP address
   *  of the listening socket (INADDR_ANY).
   *
   *  @param port The port to listen on.
   *  @param backlog The length of the incoming connection queue.  Some
   *      platforms (e.g., Win32) will silently cap this value at 5.
   *  @param isBlocking whether or not this socket is blocking.  Blocking
   *      listening sockets will create blocking connected sockets,
   *      non-blocking listening sockets will create non-blocking connected
   *      sockets.
   */
  ESFListeningTCPSocket(ESFUInt16 port, int backlog, bool isBlocking);

  /** Construct a new ESFListeningTCPSocket instance.  This socket will
   *  listen on the port and ip address specified in the ESFSocketAddress
   *  argument.
   *
   *  @param address The address (port and IP address) to listen on.
   *  @param backlog The length of the incoming connection queue.  Some
   *      platforms (e.g., Win32) will silently cap this value at 5.
   *  @param isBlocking whether or not this socket is blocking.  Blocking
   *      listening sockets will create blocking connected sockets,
   *      non-blocking listening sockets will create non-blocking connected
   *      sockets.
   */
  ESFListeningTCPSocket(ESFSocketAddress &address, int backlog,
                        bool isBlocking);

  /** Destroy the listening socket.  Will close the socket if it has not
   *  already been closed.
   */
  virtual ~ESFListeningTCPSocket();

  /** Bind the socket to its IP address and port.
   *
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  ESFError bind();

  /** Start listening.  Once this has been called, the socket library can
   *  start placing incoming connections in the incoming connection queue.
   *
   *  @return ESF_SUCCESS if successful, another error otherwise.
   */
  ESFError listen();

  /** Attempt to accept a new connected socket.  If this is a blocking socket,
   *  this method will block until a new connection is established.  If this
   *  is a non-blocking socket, the function will return immediately whether
   *  or not a new connected socket was accepted.
   *
   *  @param acceptData This object will be populated with information about
   *      the new connection.  New ESFConnectedTCPSockets can be constructed
   *      from it.
   *  @return ESF_SUCCESS if a new connected socket was created, ESF_AGAIN
   *      if this is a non-blocking socket and there were no new connections,
   *      or another error code if an error occured on this socket.
   */
  ESFError accept(AcceptData *data);

  /** Get the socket address of the listening socket.
   *
   *  @return the listening socket's address.
   */
  const ESFSocketAddress &getListeningAddress() const;

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
  //  Disabled
  ESFListeningTCPSocket(const ESFListeningTCPSocket &socket);
  ESFListeningTCPSocket &operator=(const ESFListeningTCPSocket &);

  int _backlog;
  ESFSocketAddress _listeningAddress;
};

#endif /* ! ESF_LISTENING_TCP_SOCKET_H */
