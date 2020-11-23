#ifndef ES_EPHEMERAL_LISTENER_H
#include <ESEphemeralListener.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ES {

EphemeralListener::EphemeralListener(const char *name)
    : ListeningSocket(name, ESB::SocketAddress("0.0.0.0", 0, ESB::SocketAddress::TCP), ESB_UINT16_MAX) {
  ESB::Error error = bind();
  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "[%s] cannot bind to ephemeral port", name);
    assert(!"Cannot bind to ephemeral port");
    return;
  }

  ESB_LOG_NOTICE("[%s] bound to port %u", name, listeningAddress().port());
}

EphemeralListener::~EphemeralListener() {}

ESB::SocketAddress EphemeralListener::localDestination() {
  ESB::SocketAddress originAddress("127.0.0.1", listeningAddress().port(), ESB::SocketAddress::TransportType::TCP);
  return originAddress;
}

}  // namespace ES
