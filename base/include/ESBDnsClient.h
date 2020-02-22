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

  /** TODO modify caller
    unsigned char hostname[1024];
    hostname[0] = 0;
    ESFUInt16 port = 0;
    bool isSecure = false;

    Error error =
        request->parsePeerAddress(hostname, sizeof(hostname), &port, &isSecure);

    if (ESF_SUCCESS != error) {
      if (_logger->isLoggable(ESFLogger::Warning)) {
        _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                     "[resolver] Cannot extract hostname from request");
      }

      return error;
    }
  */

  virtual Error resolve(SocketAddress *address, const unsigned char *hostname,
                        UInt16 port = 0, bool isSecure = false) = 0;

 private:
  // Disabled
  DnsClient(const DnsClient &);
  void operator=(const DnsClient &);
};

}  // namespace ESB

#endif
