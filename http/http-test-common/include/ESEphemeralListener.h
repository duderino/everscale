#ifndef ES_EPHEMERAL_LISTENER_H
#define ES_EPHEMERAL_LISTENER_H

#ifndef ESB_LISTENING_TCP_SOCKET_H
#include <ESBListeningTCPSocket.h>
#endif

namespace ES {

class EphemeralListener : public ESB::ListeningTCPSocket {
 public:
  EphemeralListener(const char *name);

  virtual ~EphemeralListener();

  ESB::SocketAddress localDestination();

 private:
  // disabled
  EphemeralListener(const EphemeralListener &socket);
  EphemeralListener &operator=(const EphemeralListener &socket);
};

}  // namespace ES

#endif
