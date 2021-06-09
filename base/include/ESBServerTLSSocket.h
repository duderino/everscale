#ifndef ESB_SERVER_TLS_SOCKET_H
#define ESB_SERVER_TLS_SOCKET_H

#ifndef ESB_TLS_SOCKET_H
#include <ESBTLSSocket.h>
#endif

#ifndef ESB_SERVER_TLS_CONTEXT_INDEX_H
#include <ESBServerTLSContextIndex.h>
#endif

namespace ESB {

/** ServerTLSSocket are used to connect to other SSL/TLS endpoints and
 *  send and receive data across the resulting channel.
 *
 *  @ingroup network
 */
class ServerTLSSocket : public TLSSocket {
 public:
  /** Construct a new server socket.
   *
   * @param acceptState init parameters created by the ListeningSocket
   * @param namePrefix A name prefix to be incorporated into log messages
   * @param contextIndex an index of TLS contexts to be used in SNI serving
   */
  ServerTLSSocket(const Socket::State &acceptState, const char *namePrefix, ServerTLSContextIndex &contextIndex);

  /** Destroy the socket.  Will close the socket if it has not
   *  already been closed.
   */
  virtual ~ServerTLSSocket();

  virtual const SocketAddress &peerAddress() const;
  virtual const void *key() const;

 protected:
  virtual Error startHandshake();

 private:
  ServerTLSContextIndex &_contextIndex;
  SocketAddress _peerAddress;

  ESB_DEFAULT_FUNCS(ServerTLSSocket);
};

}  // namespace ESB

#endif
