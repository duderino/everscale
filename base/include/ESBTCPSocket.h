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

#ifndef ESB_EMBEDDED_MAP_ELEMENT_H
#include <ESBEmbeddedMapElement.h>
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
   *  constructed from these, and they can be stored in a connection pool.
   */
  class State : public EmbeddedMapElement {
   public:
    State();
    State(bool isBlocking, SOCKET sockFd, const SocketAddress &listeningAddress, const SocketAddress &peerAddress,
          CleanupHandler *cleanupHandler = NULL);
    virtual ~State();

    inline bool isBlocking() const { return _isBlocking; }

    inline SOCKET socketDescriptor() const { return _socketDescriptor; }

    inline const SocketAddress &listeningAddress() const { return _listeningAddress; }

    inline const SocketAddress &peerAddress() const { return _peerAddress; }

    inline SocketAddress &peerAddress() { return _peerAddress; }

    inline void setIsBlocking(bool isBlocking) { _isBlocking = isBlocking; }

    inline void setSocketDescriptor(SOCKET sockFd) { _socketDescriptor = sockFd; }

    inline void setListeningAddress(const SocketAddress &listeningAddress) { _listeningAddress = listeningAddress; }

    inline void setPeerAddress(const SocketAddress &peerAddress) { _peerAddress = peerAddress; }

    virtual const void *key() const;

    virtual CleanupHandler *cleanupHandler();

   private:
    // Disabled
    State(const State &state);
    State &operator=(const State &state);

    bool _isBlocking;
    SOCKET _socketDescriptor;
    SocketAddress _listeningAddress;
    SocketAddress _peerAddress;
    CleanupHandler *_cleanupHandler;
  };

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
  TCPSocket(const State &state);

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
  virtual Error reset(const State &acceptData);

  /** Close the socket.
   */
  virtual void close();

  /** Get a presentation address for use in log messages.  For connected sockets
   * this will be the peer's ipaddr+port.  For listening sockets, this will be
   * the listening socket's ipaddr+port.
   *
   * @return The presentation address.
   */
  virtual const char *logAddress() const = 0;

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

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

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
