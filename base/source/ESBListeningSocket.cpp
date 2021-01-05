#ifndef ESB_LISTENING_SOCKET_H
#include <ESBListeningSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined HAVE_SYS_IOCTL_H && defined USE_IOCTL_FOR_NONBLOCK
#include <sys/ioctl.h>
#endif

#if defined HAVE_FCNTL_H && defined USE_FCNTL_FOR_NONBLOCK
#include <fcntl.h>
#endif

namespace ESB {

ListeningSocket::ListeningSocket(const char *namePrefix)
    : Socket(), _backlog(42), _state(SocketState::CLOSED), _listeningAddress() {
  formatPrefix(namePrefix);
}

ListeningSocket::ListeningSocket(const char *namePrefix, const SocketAddress &address, int backlog, bool isBlocking)
    : Socket(isBlocking), _backlog(backlog), _state(SocketState::CLOSED), _listeningAddress(address) {
  formatPrefix(namePrefix);
}

ListeningSocket::~ListeningSocket() {}

ListeningSocket::ListeningSocket(const ListeningSocket &socket) { duplicate(socket); }

ListeningSocket &ListeningSocket::operator=(const ListeningSocket &socket) {
  duplicate(socket);
  return *this;
}

Error ListeningSocket::duplicate(const ListeningSocket &socket) {
  _isBlocking = socket._isBlocking;
  _backlog = socket._backlog;
  _listeningAddress = socket._listeningAddress;

#ifdef HAVE_SO_REUSEPORT
  Error error = bind();
  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "[%s] cannot SO_REUSEPORT re-bind port %u", name(), _listeningAddress.port());
    return error;
  }

  error = listen();
  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "[%s] cannot SO_REUSEPORT re-listen port %u", name(), _listeningAddress.port());
    close();
    return error;
  }
#elif defined HAVE_DUP
  _state = socket._state;
  _sockFd = dup(socket._sockFd);

  if (0 > _sockFd) {
    ESB::Error error = LastError();
    ESB_LOG_ERROR_ERRNO(error, "Cannot duplicate listening socket on port %u", _listeningAddress.port());
    _sockFd = INVALID_SOCKET;
    return error;
  }
#else
#error "dup() or equivalent is required"
#endif

  const char *p = socket._logAddress;
  char *q = _logAddress;
  for (; *p && *p != ':'; ++p, ++q) {
    *q = *p;
  }

  if (*p == ':') {
    *q++ = ':';
    _listeningAddress.logAddress(q, sizeof(_logAddress) - (q - _logAddress), _sockFd);
  } else {
    *q = 0;
  }

  ESB_LOG_NOTICE("[%s] duplicated listening socket on port %u", name(), _listeningAddress.port());

  return ESB_SUCCESS;
}

Error ListeningSocket::bind() {
  if (SocketState::CLOSED != _state) {
    return ESB_SUCCESS;
  }

#ifdef HAVE_SOCKET
  _sockFd = socket(AF_INET, SOCK_STREAM, 0);
#else
#error "socket or equivalent is required"
#endif

  if (INVALID_SOCKET == _sockFd) {
    return LastError();
  }

  ESB::Error error = setBlocking(_isBlocking);

  if (ESB_SUCCESS != error) {
    close();
    return error;
  }

#ifdef HAVE_SETSOCKOPT
  int value = 1;
  if (SOCKET_ERROR == setsockopt(_sockFd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value))) {
    close();
    return LastError();
  }
#ifdef HAVE_SO_REUSEPORT
  value = 1;
  if (SOCKET_ERROR == setsockopt(_sockFd, SOL_SOCKET, SO_REUSEPORT, &value, sizeof(value))) {
    close();
    return LastError();
  }
#endif
#else
#error "setsockopt or equivalent is required"
#endif

#if defined HAVE_BIND && defined HAVE_STRUCT_SOCKADDR
  if (SOCKET_ERROR ==
      ::bind(_sockFd, (sockaddr *)_listeningAddress.primitiveAddress(), sizeof(SocketAddress::Address))) {
    close();
    return LastError();
  }
#else
#error "bind and sockaddr or equivalent is required."
#endif

  if (0 != _listeningAddress.port()) {
    _state = SocketState::BOUND;
    return ESB_SUCCESS;
  }

  // If we bound to an ephemeral port, determine the assigned port.

  SocketAddress::Address addr;

#ifdef HAVE_SOCKLEN_T
  socklen_t length = sizeof(addr);
#else
  ESB::UInt32 length = sizeof(addr);
#endif

#if defined HAVE_GETSOCKNAME
  if (SOCKET_ERROR == ::getsockname(_sockFd, (sockaddr *)&addr, &length)) {
    close();
    return LastError();
  }
  assert(length == sizeof(addr));
#else
#error "getsockname or equivalent is required."
#endif

  _listeningAddress.updatePrimitiveAddress(&addr);

  int len = strlen(_logAddress);
  _listeningAddress.logAddress(_logAddress + len, sizeof(_logAddress) - len, _sockFd);

  _state = SocketState::BOUND;
  return ESB_SUCCESS;
}

Error ListeningSocket::listen() {
  switch (_state) {
    case BOUND:
      break;
    case LISTENING:
      return ESB_SUCCESS;
    case CLOSED:
    default:
      return ESB_INVALID_STATE;
  }

#ifdef HAVE_LISTEN
  if (SOCKET_ERROR == ::listen(_sockFd, _backlog)) {
    return LastError();
  }
#else
#error "listen or equivalent is required."
#endif

  _state = SocketState::LISTENING;

  return ESB_SUCCESS;
}

void ListeningSocket::close() {
  Socket::close();
  _state = SocketState::CLOSED;
}

Error ListeningSocket::accept(State *data) {
  if (SocketState::LISTENING != _state) {
    assert(!"Attempted to accept on an invalid socket.");
    return ESB_INVALID_STATE;
  }

  if (!data) {
    return ESB_NULL_POINTER;
  }

#ifdef HAVE_SOCKLEN_T
  socklen_t addressSize;
#else
  int addressSize;
#endif

#if defined HAVE_ACCEPT && defined HAVE_STRUCT_SOCKADDR
  addressSize = sizeof(SocketAddress::Address);
  data->setSocketDescriptor(::accept(_sockFd, (sockaddr *)data->peerAddress().primitiveAddress(), &addressSize));
#else
#error "accept and sockaddr or equivalent is required."
#endif

  if (INVALID_SOCKET == data->socketDescriptor()) {
    return LastError();
  }

  if (!_isBlocking) {
    Error error = Socket::SetBlocking(data->socketDescriptor(), _isBlocking);

    if (ESB_SUCCESS != error) {
      Socket::Close(data->socketDescriptor());
      data->setSocketDescriptor(-1);
      return error;
    }
  }

  SocketAddress peerAddress;

#if defined HAVE_GETSOCKNAME
  addressSize = sizeof(SocketAddress::Address);
  if (SOCKET_ERROR == ::getsockname(data->socketDescriptor(), (sockaddr *) peerAddress.primitiveAddress(), &addressSize)) {
    Socket::Close(data->socketDescriptor());
    return LastError();
  }
#else
#error "getsockname or equivalent is required"
#endif

  peerAddress.setType(_listeningAddress.type());

  data->setPeerAddress(peerAddress);
  data->setListeningAddress(_listeningAddress);
  data->setIsBlocking(_isBlocking);

  return ESB_SUCCESS;
}

const SocketAddress &ListeningSocket::listeningAddress() const { return _listeningAddress; }

const char *ListeningSocket::name() const { return _logAddress; }

void ListeningSocket::formatPrefix(const char *namePrefix) {
  int desired = snprintf(_logAddress, ESB_NAME_PREFIX_SIZE, "%s:", namePrefix);
  _logAddress[MIN(ESB_NAME_PREFIX_SIZE - 2, desired - 1)] = ':';
  _logAddress[MIN(ESB_NAME_PREFIX_SIZE - 1, desired)] = 0;
}

const void *ListeningSocket::key() const { return &_listeningAddress; }

}  // namespace ESB
