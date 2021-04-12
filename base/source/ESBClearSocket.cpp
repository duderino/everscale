#ifndef ESB_CLEAR_SOCKET_H
#include <ESBClearSocket.h>
#endif

namespace ESB {

ClearSocket::ClearSocket(const Socket::State &acceptState, const char *namePrefix)
    : ConnectedSocket(acceptState, namePrefix),
      _peerAddress(acceptState.peerAddress()),
      _key(_peerAddress, SocketKey::CLEAR_OBJECT, this) {}

ClearSocket::ClearSocket(const SocketAddress &peer, const char *namePrefix, bool isBlocking)
    : ConnectedSocket(namePrefix, isBlocking), _peerAddress(peer), _key(_peerAddress, SocketKey::CLEAR_OBJECT, this) {}

ClearSocket::~ClearSocket() {}

const void *ClearSocket::key() const { return &_key; }

const SocketAddress &ClearSocket::peerAddress() const { return _peerAddress; }

bool ClearSocket::secure() const { return false; }

}  // namespace ESB
