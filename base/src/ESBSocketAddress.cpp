#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ESB_SOCKET_H
#include <ESBSocket.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

namespace ESB {

SocketAddress::SocketAddress() : _magic(0) {
#ifdef HAVE_MEMSET
  memset(&_address, 0, sizeof(Address));
#else
#error "memset equivalent is required."
#endif

#ifdef HAVE_STRUCT_SOCKADDR_IN
  _address.sin_family = AF_INET;
  _address.sin_port = 0;
#if defined HAVE_HTONL && defined HAVE_INADDR_ANY
  _address.sin_addr.s_addr = htonl(INADDR_ANY);
#else
#error "htonl or equivalent is required."
#endif

#else /* ! HAVE_STRUCT_SOCKADDR_IN */
#error "sockaddr_in or equivalent is required"
#endif

  _transport = NONE;
}

SocketAddress::SocketAddress(const char *dottedIp, UInt16 port,
                             TransportType transport)
    : _magic(0) {
#ifdef HAVE_MEMSET
  memset(&_address, 0, sizeof(Address));
#else
#error "memset equivalent is required."
#endif

  _magic = ESB_MAGIC;

#ifdef HAVE_STRUCT_SOCKADDR_IN

  _address.sin_family = AF_INET;

#if defined HAVE_INET_PTON

  if (1 != inet_pton(AF_INET, dottedIp, &_address.sin_addr)) {
    _magic = 0;
  }

#elif defined HAVE_INET_ADDR && defined HAVE_INADDR_NONE

  UInt32 ip = inet_addr(dottedIp);

  if (INADDR_NONE == ip) {
    _magic = 0;
  } else {
    _address.sin_addr.s_addr = ip;
  }

#else
#error "inet_pton equivalent is required."
#endif

#else /* ! HAVE_STRUCT_SOCKADDR_IN */
#error "sockaddr_in or equivalent is required"
#endif

#ifdef HAVE_HTONS
  _address.sin_port = htons(port);
#else
#error "htons equivalent is required."
#endif

  _transport = transport;

  switch (transport) {
    case TCP:
    case UDP:
    case TLS:
      break;

    default:
      _magic = 0;
  }
}

SocketAddress::SocketAddress(const SocketAddress &address) {
#ifdef HAVE_MEMCPY
  memcpy(&_address, &address._address, sizeof(Address));
#else
#error "memcpy equivalent is required"
#endif

  _transport = address._transport;
  _magic = address._magic;
}

SocketAddress &SocketAddress::operator=(const SocketAddress &address) {
#ifdef HAVE_MEMCPY
  memcpy(&_address, &address._address, sizeof(Address));
#else
#error "memcpy equivalent is required"
#endif

  _transport = address._transport;
  _magic = address._magic;

  return *this;
}

SocketAddress::Address *SocketAddress::getAddress() { return &_address; }

void SocketAddress::getIPAddress(char *address, int size) const {
  if (!address || 16 > size) {
    return;
  }

#ifdef HAVE_STRUCT_SOCKADDR_IN

#if defined HAVE_INET_NTOP
  inet_ntop(AF_INET, &_address.sin_addr, address, size);
#elif defined HAVE_INET_NTOA && defined HAVE_STRNCPY
  // Solaris actually uses thread-specific data to make this safe.
  strncpy(address, inet_ntoa(_address.sin_addr), size);
#else
#error "inet_ntop and sockaddr_in or equivalent is required"
#endif

#else /* ! HAVE_STRUCT_SOCKADDR_IN */
#error "sockaddr_in or equivalent is required"
#endif
}

UInt16 SocketAddress::getPort() const {
#if defined HAVE_NTOHS && defined HAVE_STRUCT_SOCKADDR_IN
  return ntohs(_address.sin_port);
#else
#error "ntohs and sockaddr_in or equivalent is required"
#endif
}

void SocketAddress::setPort(UInt16 port) {
#if defined HAVE_HTONS && defined HAVE_STRUCT_SOCKADDR_IN
  _address.sin_port = htons(port);
#else
#error "htons and sockaddr_in or equivalent is required"
#endif
}

SocketAddress::TransportType SocketAddress::getTransport() const {
  return _transport;
}

void SocketAddress::setTransport(TransportType transport) {
  _transport = transport;

  switch (_transport) {
    case TCP:
    case UDP:
    case TLS:
      break;

    default:
      _magic = 0;
  }
}

bool SocketAddress::isValid() { return ESB_MAGIC == _magic; }

}  // namespace ESB
