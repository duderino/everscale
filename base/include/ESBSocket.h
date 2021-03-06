#ifndef ESB_SOCKET_H
#define ESB_SOCKET_H

#ifndef ESB_SOCKET_TYPE_H
#include <ESBSocketType.h>
#endif

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ESB_EMBEDDED_MAP_ELEMENT_H
#include <ESBEmbeddedMapElement.h>
#endif

namespace ESB {

/** Socket is a generic base class for connected and listening sockets.
 *
 *  @ingroup network
 */
class Socket : public EmbeddedMapElement {
 public:
  /** ListeningTCPSockets create these and ConnectedTCPSockets can be constructed from these.
   */
  class State {
   public:
    State();
    State(bool isBlocking, SOCKET sockFd, const SocketAddress &localAddress, const SocketAddress &peerAddress);
    virtual ~State();

    inline bool isBlocking() const { return _isBlocking; }

    inline SOCKET socketDescriptor() const { return _socketDescriptor; }

    inline const SocketAddress &localAddress() const { return _localAddress; }

    inline const SocketAddress &peerAddress() const { return _peerAddress; }

    inline SocketAddress &peerAddress() { return _peerAddress; }

    inline void setIsBlocking(bool isBlocking) { _isBlocking = isBlocking; }

    inline void setSocketDescriptor(SOCKET sockFd) { _socketDescriptor = sockFd; }

    inline void setListeningAddress(const SocketAddress &listeningAddress) { _localAddress = listeningAddress; }

    inline void setPeerAddress(const SocketAddress &peerAddress) { _peerAddress = peerAddress; }

   private:
    bool _isBlocking;
    SOCKET _socketDescriptor;
    SocketAddress _localAddress;
    SocketAddress _peerAddress;

    ESB_DISABLE_AUTO_COPY(State);
  };

  /** Construct a server Socket
   *
   * @param acceptState init parameters created by the ListeningSocket
   */
  Socket(const State &acceptState);

  /** Construct a client Socket.
   *
   *  @param isBlocking true if this socket is a blocking socket.
   */
  Socket(bool isBlocking = false);

  /** Destroy the socket.  Will close the socket if it has not
   *  already been closed.
   */
  virtual ~Socket();

  virtual CleanupHandler *cleanupHandler();

  /** Close the socket.
   */
  virtual void close();

  /** Get a presentation address for use in log messages.  For connected sockets
   * this will be the peer's ipaddr+port.  For listening sockets, this will be
   * the listening socket's ipaddr+port.
   *
   * @return The presentation address.
   */
  virtual const char *name() const = 0;

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
  inline SOCKET socketDescriptor() const { return _sockFd; }

  /** Get and clear the last error on this socket.
   *
   *  @return the last error that occurred on this socket or ESB_SUCCESS if
   *      no error has occurred.
   */
  inline Error lastError() { return LastSocketError(_sockFd); }

  /** Get and clear the last error on a socket descriptor.
   *
   *  @param socket The socked descriptor
   *  @return the last error that occurred on the socket descriptor or
   * ESB_SUCCESS if no error has occurred.
   */
  static Error LastSocketError(SOCKET socket);

 protected:
  bool _isBlocking;
  SOCKET _sockFd;

  ESB_DEFAULT_FUNCS(Socket);
};

/**
 * Sockets can be stored in connection pools using this key.
 */
class SocketKey {
 public:
  enum Tag { CLEAR_KEY = 0, CLEAR_OBJECT = 1, TLS_KEY = 2, TLS_OBJECT = 3 };

  SocketKey(const SocketAddress &peerAddress, Tag tag, void *context)
      : _peerAddress(peerAddress), _tag(tag), _context(context) {}

  const SocketAddress &_peerAddress;
  Tag _tag;
  void *_context;
};

}  // namespace ESB

#endif
