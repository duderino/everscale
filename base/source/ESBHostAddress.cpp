#ifndef ESB_HOST_ADDRESS_H
#include <ESBHostAddress.h>
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

#ifdef HAVE_SYS_HOST_H
#include <sys/socket.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

namespace ESB {

HostAddress::HostAddress() : SocketAddress() { _host[0] = 0; }

HostAddress::HostAddress(const char *host, const char *presentation, UInt16 port, TransportType transport)
    : SocketAddress(presentation, port, transport) {
  setHost(host);
}

HostAddress::HostAddress(const char *host, SocketAddress &address) {
  SocketAddress::operator=(address);
  setHost(host);
}

HostAddress::HostAddress(const HostAddress &address) {
  SocketAddress::operator=(address);
  setHost(address.host());
}

HostAddress &HostAddress::operator=(const HostAddress &address) {
  SocketAddress::operator=(address);
  setHost(address.host());
  return *this;
}

const char *HostAddress::host() const { return _host; }

}  // namespace ESB
