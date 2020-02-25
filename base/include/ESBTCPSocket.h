#ifndef ESB_TCP_SOCKET_H
#define ESB_TCP_SOCKET_H

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

namespace ESB {

/** TCPSocket is a generic base class for connected and listening tcp
 *  sockets.
 *
 *  @ingroup network
 */
class TCPSocket {
 public:
  /** ListeningTCPSockets create these, ConnectedTCPSockets can be
   *  constructed from these.  The sole function of this struct is to
   *  transfer data between those two classes.
   */
  typedef struct {
    bool _isBlocking;
    SOCKET _sockFd;
    SocketAddress _listeningAddress;
    SocketAddress _peerAddress;
  } AcceptData;

  /** Construct an uninitialized TCPSocket.
   */
  TCPSocket();

  /** Construct a new TCPSocket.
   *
   *  @param isBlocking true if this socket is a blocking socket.
   */
  TCPSocket(bool isBlocking);

  /** Construct a new server TCPSocket.
   *
   * @param acceptData An object created popupated by ListeningTCPSockets
   *  when accepting a new connection.
   */
  TCPSocket(AcceptData *acceptData);

  /** Destroy the socket.  Will close the socket if it has not
   *  already been closed.
   */
  virtual ~TCPSocket();

  /** Reset a tcp socket.  If the socket is currently open, this will close
   *  it as a side-effect.
   *
   * @param acceptData An object created popupated by ListeningTCPSockets
   *  when accepting a new connection.
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error reset(AcceptData *acceptData);

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
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error setBlocking(bool isBlocking);

  /** Set a socket descriptor's blocking/non-blocking property.
   *
   *  @param isBlocking Whether or not the socket is blocking.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  static Error SetBlocking(SOCKET sockFd, bool isBlocking);

  /** Get the socket's socket descriptor.
   *
   *  @return the socket descriptor
   */
  inline SOCKET getSocketDescriptor() const { return _sockFd; }

  /** Get and clear the last error on this socket.
   *
   *  @return the last error that occurred on this socket or ESB_SUCCESS if
   *      no error has occurred.
   */
  inline Error getLastError() { return GetLastSocketError(_sockFd); }

  /** Get and clear the last error on a socket descriptor.
   *
   *  @param socket The socked descriptor
   *  @return the last error that occurred on the socket descriptor or
   * ESB_SUCCESS if no error has occurred.
   */
  static Error GetLastSocketError(SOCKET socket);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

 protected:
  bool _isBlocking;
  SOCKET _sockFd;

 private:
  // Disabled
  TCPSocket(const TCPSocket &socket);
  TCPSocket &operator=(const TCPSocket &);
};

}  // namespace ESB

#endif
