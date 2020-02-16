/** @file ESFTCPSocket.h
 *  @brief An abstract base class with code common to both
 *      ESFConnectedTCPSocket and ESFListeningTCPSocket instances
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_TCP_SOCKET_H
#define ESF_TCP_SOCKET_H

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

/** ESFTCPSocket is a generic base class for connected and listening tcp
 *  sockets.
 *
 *  @ingroup network
 */
class ESFTCPSocket {
 public:
  /** ESFListeningTCPSockets create these, ESFConnectedTCPSockets can be
   *  constructed from these.  The sole function of this struct is to
   *  transfer data between those two classes.
   */
  typedef struct {
    bool _isBlocking;
    SOCKET _sockFd;
    ESFSocketAddress _listeningAddress;
    ESFSocketAddress _peerAddress;
  } AcceptData;

  /** Construct an uninitialized ESFTCPSocket.
   */
  ESFTCPSocket();

  /** Construct a new ESFTCPSocket.
   *
   *  @param isBlocking true if this socket is a blocking socket.
   */
  ESFTCPSocket(bool isBlocking);

  /** Construct a new server ESFTCPSocket.
   *
   * @param acceptData An object created popupated by ESFListeningTCPSockets
   *  when accepting a new connection.
   */
  ESFTCPSocket(AcceptData *acceptData);

  /** Destroy the socket.  Will close the socket if it has not
   *  already been closed.
   */
  virtual ~ESFTCPSocket();

  /** Reset a tcp socket.  If the socket is currently open, this will close
   *  it as a side-effect.
   *
   * @param acceptData An object created popupated by ESFListeningTCPSockets
   *  when accepting a new connection.
   * @return ESF_SUCCESS if successful, another error code otherwise.
   */
  virtual ESFError reset(AcceptData *acceptData);

  /** Close the socket.
   */
  virtual void close();

  /** Close a socket descriptor
   *
   * @param socket The socket descriptor to close
   */
  static void Close(SOCKET socket);

  /** Determine whether or not this socket is a blocking socket.
   *
   *  @return true if this socket is a blocking socket, false otherwise.
   */
  inline bool isBlocking() const { return _isBlocking; }

  /** Set the socket's blocking/non-blocking property.
   *
   *  @param isBlocking Whether or not the socket is blocking.
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  ESFError setBlocking(bool isBlocking);

  /** Set a socket descriptor's blocking/non-blocking property.
   *
   *  @param isBlocking Whether or not the socket is blocking.
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  static ESFError SetBlocking(SOCKET sockFd, bool isBlocking);

  /** Get the socket's socket descriptor.
   *
   *  @return the socket descriptor
   */
  inline SOCKET getSocketDescriptor() const { return _sockFd; }

  /** Get and clear the last error on this socket.
   *
   *  @return the last error that occurred on this socket or ESF_SUCCESS if
   *      no error has occurred.
   */
  inline ESFError getLastError() { return GetLastError(_sockFd); }

  /** Get and clear the last error on a socket descriptor.
   *
   *  @param socket The socked descriptor
   *  @return the last error that occurred on the socket descriptor or
   * ESF_SUCCESS if no error has occurred.
   */
  static ESFError GetLastError(SOCKET socket);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

 protected:
  bool _isBlocking;
  SOCKET _sockFd;

 private:
  // Disabled
  ESFTCPSocket(const ESFTCPSocket &socket);
  ESFTCPSocket &operator=(const ESFTCPSocket &);
};

#endif /* ! ESF_TCP_SOCKET_H */
