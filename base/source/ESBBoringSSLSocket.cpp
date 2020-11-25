#ifndef ESB_BORING_SSL_SOCKET_H
#include <ESBBoringSSLSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ESB {

BoringSSLSocket::BoringSSLSocket(const char *namePrefix, const char *nameSuffix, bool isBlocking)
    : ConnectedSocket(namePrefix, nameSuffix, isBlocking) {}

BoringSSLSocket::BoringSSLSocket(const char *namePrefix, const char *nameSuffix, const SocketAddress &peer,
                                 bool isBlocking)
    : ConnectedSocket(namePrefix, nameSuffix, peer, isBlocking) {}

BoringSSLSocket::~BoringSSLSocket() {}

bool BoringSSLSocket::secure() { return true; }

}  // namespace ESB
