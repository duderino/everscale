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

  virtual Error resolve(SocketAddress *address, const unsigned char *hostname,
                        UInt16 port = 0, bool isSecure = false);

  inline void *operator new(size_t size, Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 private:
  // Disabled
  SystemDnsClient(const SystemDnsClient &);
  void operator=(const SystemDnsClient &);
};

}  // namespace ESB

#endif
