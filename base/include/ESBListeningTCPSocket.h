#ifndef ESB_LISTENING_TCP_SOCKET_H
#define ESB_LISTENING_TCP_SOCKET_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_SOCKET_H
#include <ESBSocket.h>
#endif

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ESB_TCP_SOCKET_H
#include <ESBTCPSocket.h>
#endif

#ifndef ESB_CONNECTED_TCP_SOCKET_H
#include <ESBConnectedTCPSocket.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

namespace ESB {

/** @todo allow binding to a SocketAddress - bind() with no args will
 *  bind to INADDR_ANY
 */

/** ListeningTCPSockets are used to receive connections from other TCP
 *  endpoints.  New connections are wrapped in ConnectedTCPSockets.
 *
 *  @ingroup network
 */
class ListeningTCPSocket : public TCPSocket {
 public:
  enum SocketState { CLOSED = 0, BOUND = 1, LISTENING = 2 };

  /**
   * Constructor
   */
  ListeningTCPSocket(const char *namePrefix);

  /** Construct a new ListeningTCPSocket instance.  This socket will
   *  listen on the specified port and let the kernel choose the IP address
   *  of the listening socket (INADDR_ANY).
   *
   *  @param port The port to listen on.  If 0 then pick an ephemeral port.
   *  @param backlog The length of the incoming connection queue.  Some
   *      platforms (e.g., Win32) will silently cap this value at 5.
   *  @param isBlocking whether or not this socket is blocking.  Blocking
   *      listening sockets will create blocking connected sockets,
   *      non-blocking listening sockets will create non-blocking connected
   *      sockets.
   */
  ListeningTCPSocket(const char *namePrefix, UInt16 port, int backlog, bool isBlocking = false);

  /** Construct a new ListeningTCPSocket instance.  This socket will
   *  listen on the port and ip address specified in the SocketAddress
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
  ListeningTCPSocket(const char *namePrefix, const SocketAddress &address, int backlog, bool isBlocking = false);

  /**
   * Duplicate an existing bound and listening socket by creating a new file
   * descriptor bound to the same port as the original.
   *
   * @param socket The socket to duplicate
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error duplicate(const ListeningTCPSocket &socket);

  /** Destroy the listening socket.  Will close the socket if it has not
   *  already been closed.
   */
  virtual ~ListeningTCPSocket();

  /* Get the listening socket's ipaddr+port in human-friendly presentation
   * format.
   *
   * @return The presentation address.
   */
  virtual const char *name() const;

  /** Bind the socket to its IP address and port.
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error bind();

  /** Start listening.  Once this has been called, the socket library can
   *  start placing incoming connections in the incoming connection queue.
   *
   *  @return ESB_SUCCESS if successful, another error otherwise.
   */
  Error listen();

  /** Attempt to accept a new connected socket.  If this is a blocking socket,
   *  this method will block until a new connection is established.  If this
   *  is a non-blocking socket, the function will return immediately whether
   *  or not a new connected socket was accepted.
   *
   *  @param acceptData This object will be populated with information about
   *      the new connection.  New ConnectedTCPSockets can be constructed
   *      from it.
   *  @return ESB_SUCCESS if a new connected socket was created, ESB_AGAIN
   *      if this is a non-blocking socket and there were no new connections,
   *      or another error code if an error occured on this socket.
   */
  Error accept(State *data);

  /** Close the socket.
   */
  virtual void close();

  /** Get the socket address of the listening socket.
   *
   *  @return the listening socket's address.
   */
  const SocketAddress &listeningAddress() const;

  inline int backlog() const { return _backlog; }

  inline SocketState state() const { return _state; }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  // disabled
  ListeningTCPSocket(const ListeningTCPSocket &socket);
  ListeningTCPSocket &operator=(const ListeningTCPSocket &socket);

  int _backlog;
  SocketState _state;
  SocketAddress _listeningAddress;
  mutable char _logAddress[ESB_LOG_ADDRESS_SIZE];
};

}  // namespace ESB

#endif
