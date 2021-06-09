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

  ESB_DEFAULT_FUNCS(EphemeralListener);
};

}  // namespace ES

#endif
