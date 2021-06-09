#ifndef ESB_SYSTEM_DNS_CLIENT_H
#define ESB_SYSTEM_DNS_CLIENT_H

#ifndef ESB_DNS_CLIENT_H
#include <ESBDnsClient.h>
#endif

namespace ESB {

class SystemDnsClient : public DnsClient {
 public:
  SystemDnsClient();

  virtual ~SystemDnsClient();

  virtual Error resolve(SocketAddress &address, const char *hostname, UInt16 port = 0, bool isSecure = false);

  ESB_DEFAULT_FUNCS(SystemDnsClient);
};

}  // namespace ESB

#endif
