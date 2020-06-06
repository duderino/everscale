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

  virtual Error resolve(SocketAddress &address, const unsigned char *hostname, UInt16 port = 0,
                        bool isSecure = false) = 0;

 private:
  // Disabled
  DnsClient(const DnsClient &);
  void operator=(const DnsClient &);
};

}  // namespace ESB

#endif
