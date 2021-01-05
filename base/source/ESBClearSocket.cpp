#ifndef ESB_CLEAR_SOCKET_H
#include <ESBClearSocket.h>
#endif

namespace ESB {

ClearSocket::ClearSocket(const Socket::State &acceptState, const char *namePrefix)
    : ConnectedSocket(acceptState, namePrefix), _peerAddress(acceptState.peerAddress()) {}

ClearSocket::ClearSocket(const SocketAddress &peer, const char *namePrefix, bool isBlocking)
    : ConnectedSocket(namePrefix, isBlocking), _peerAddress(peer) {}

ClearSocket::~ClearSocket() {}

const void *ClearSocket::key() const { return &_peerAddress; }

const SocketAddress &ClearSocket::peerAddress() const { return _peerAddress; }

bool ClearSocket::secure() const { return false; }

}  // namespace ESB
