#ifndef ESB_DNS_CLIENT_H
#define ESB_DNS_CLIENT_H

#ifndef ESF_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

namespace ESB {

class DnsClient {
 public:
  DnsClient();

  virtual ~DnsClient();

  virtual Error resolve(SocketAddress &address, const char *hostname, UInt16 port = 0, bool isSecure = false) = 0;

  ESB_DEFAULT_FUNCS(DnsClient);
};

}  // namespace ESB

#endif
