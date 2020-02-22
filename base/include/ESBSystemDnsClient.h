#ifndef ESB_SYSTEM_DNS_CLIENT_H
#define ESB_SYSTEM_DNS_CLIENT_H

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_DNS_CLIENT_H
#include <ESBDnsClient.h>
#endif

namespace ESB {

class SystemDnsClient : public DnsClient {
 public:
  SystemDnsClient(Logger *logger);

  virtual ~SystemDnsClient();

  virtual Error resolve(SocketAddress *address, const unsigned char *hostname,
                        UInt16 port = 0, bool isSecure = false);

 private:
  // Disabled
  SystemDnsClient(const SystemDnsClient &);
  void operator=(const SystemDnsClient &);

  Logger *_logger;
};

}  // namespace ESB

#endif
