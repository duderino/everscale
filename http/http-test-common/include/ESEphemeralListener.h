#ifndef ES_EPHEMERAL_LISTENER_H
#define ES_EPHEMERAL_LISTENER_H

#ifndef ESB_LISTENING_SOCKET_H
#include <ESBListeningSocket.h>
#endif

namespace ES {

class EphemeralListener : public ESB::ListeningSocket {
 public:
  EphemeralListener(const char *name, bool secure);

  virtual ~EphemeralListener();

  ESB::SocketAddress localDestination();

 private:
  // disabled
  EphemeralListener(const EphemeralListener &socket);
  EphemeralListener &operator=(const EphemeralListener &socket);
};

}  // namespace ES

#endif
