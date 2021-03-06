#ifndef ESB_LISTENING_SOCKET_H
#define ESB_LISTENING_SOCKET_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

#ifndef ESB_SOCKET_TYPE_H
#include <ESBSocketType.h>
#endif

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ESB_SOCKET_H
#include <ESBSocket.h>
#endif

#ifndef ESB_CONNECTED_SOCKET_H
#include <ESBConnectedSocket.h>
#endif

#ifndef ESB_ANY_PORT
#define ESB_ANY_PORT 0
#endif

namespace ESB {

/** @todo allow binding to a SocketAddress - bind() with no args will
 *  bind to INADDR_ANY
 */

/** ListeningSockets are used to receive connections from other TCP
 *  endpoints.  New connections are wrapped in ConnectedTCPSockets.
 *
 *  @ingroup network
 */
class ListeningSocket : public Socket {
 public:
  enum SocketState { CLOSED = 0, BOUND = 1, LISTENING = 2 };

  /**
   * Constructor
   */
  ListeningSocket(const char *namePrefix);

  /** Construct a new ListeningSocket instance.  This socket will
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
  ListeningSocket(const char *namePrefix, const SocketAddress &address, int backlog, bool isBlocking = false);

  /**
   * Duplicate an existing bound and listening socket by creating a new file
   * descriptor bound to the same port as the original.
   *
   * @param socket The socket to duplicate
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error duplicate(const ListeningSocket &socket);

  /** Destroy the listening socket.  Will close the socket if it has not
   *  already been closed.
   */
  virtual ~ListeningSocket();

  virtual const void *key() const;

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

 private:
  void formatPrefix(const char *namePrefix);

  int _backlog;
  SocketState _state;
  SocketAddress _listeningAddress;
  // <prefix>:<ip addr>:<port>,<fd>
  mutable char _logAddress[ESB_NAME_PREFIX_SIZE + 1 + ESB_ADDRESS_PORT_SIZE + 1 + ESB_MAX_UINT32_STRING_LENGTH];

  ESB_DEFAULT_FUNCS(ListeningSocket);
};

}  // namespace ESB

#endif
