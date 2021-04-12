#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ESB_SOCKET_TYPE_H
#include <ESBSocketType.h>
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

#ifdef HAVE_STRING_H
#include <string.h>
#endif

namespace ESB {

SocketAddress::SocketAddress() {
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

SocketAddress::SocketAddress(const char *presentation, UInt16 port, TransportType transport) {
#ifdef HAVE_MEMSET
  memset(&_address, 0, sizeof(Address));
#else
#error "memset equivalent is required."
#endif

#ifdef HAVE_STRUCT_SOCKADDR_IN
  _address.sin_family = AF_INET;
#else
#error "sockaddr_in or equivalent is required"
#endif

  setAddress(presentation);
  setPort(port);
  setType(transport);
}

SocketAddress::SocketAddress(const SocketAddress &address) {
#ifdef HAVE_MEMCPY
  memcpy(&_address, &address._address, sizeof(Address));
#else
#error "memcpy equivalent is required"
#endif

  _transport = address._transport;
}

SocketAddress &SocketAddress::operator=(const SocketAddress &address) {
#ifdef HAVE_MEMCPY
  memcpy(&_address, &address._address, sizeof(Address));
#else
#error "memcpy equivalent is required"
#endif

  _transport = address._transport;

  return *this;
}

const SocketAddress::Address *SocketAddress::primitiveAddress() const { return &_address; }

SocketAddress::Address *SocketAddress::primitiveAddress() { return &_address; }

void SocketAddress::presentationAddress(char *address, int size) const {
  assert(address);
  assert(size >= ESB_IPV6_PRESENTATION_SIZE);
  if (!address || ESB_IPV6_PRESENTATION_SIZE > size) {
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

  address[ESB_IPV6_PRESENTATION_SIZE - 1] = 0;
}

int SocketAddress::logAddress(char *address, int size, int fd) const {
  assert(address);
  char buffer[ESB_IPV6_PRESENTATION_SIZE];
  presentationAddress(buffer, sizeof(buffer));

  int bytesWrittenExcludingNull = 0;

  if (INVALID_SOCKET != fd) {
    assert(size >= ESB_ADDRESS_PORT_SIZE + 1 + ESB_MAX_UINT32_STRING_LENGTH);
    if (!address || (ESB_ADDRESS_PORT_SIZE + 1 + ESB_MAX_UINT32_STRING_LENGTH) > size) {
      return 0;
    }
    bytesWrittenExcludingNull = snprintf(address, size, "%s:%u,%d", buffer, port(), fd);
    address[ESB_ADDRESS_PORT_SIZE + 1 + ESB_MAX_UINT32_STRING_LENGTH - 1] = 0;
  } else {
    assert(size >= ESB_ADDRESS_PORT_SIZE);
    if (!address || ESB_ADDRESS_PORT_SIZE > size) {
      return 0;
    }
    bytesWrittenExcludingNull = snprintf(address, size, "%s:%u", buffer, port());
    address[ESB_ADDRESS_PORT_SIZE - 1] = 0;
  }

  return bytesWrittenExcludingNull;
}

UInt16 SocketAddress::port() const {
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

SocketAddress::TransportType SocketAddress::type() const { return _transport; }

void SocketAddress::setType(TransportType transport) { _transport = transport; }

void SocketAddress::updatePrimitiveAddress(SocketAddress::Address *address) {
  if (!address) {
    return;
  }

  memcpy(&_address, address, sizeof(_address));
}

ESB::Error SocketAddress::setAddress(const char *presentation) {
#if defined HAVE_INET_PTON

  if (1 != inet_pton(AF_INET, presentation, &_address.sin_addr)) {
    return LastError();
  }

#elif defined HAVE_INET_ADDR && defined HAVE_INADDR_NONE

  UInt32 ip = inet_addr(dottedIp);

  if (INADDR_NONE == ip) {
    return LastError();
  } else {
    _address.sin_addr.s_addr = ip;
  }

#else
#error "inet_pton equivalent is required."
#endif

  return ESB_SUCCESS;
}

}  // namespace ESB
