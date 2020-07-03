#ifndef ES_HTTP_SOCKET_H
#define ES_HTTP_SOCKET_H

#ifndef ESB_MULTIPLEXED_SOCKET_H
#include <ESBMultiplexedSocket.h>
#endif

namespace ES {

/**
 * Common HTTP socket functionality shared by both HTTP Server and HTTP Client connections.
 */
class HttpSocket : public ESB::MultiplexedSocket {
 public:
  /** Constructor
   */
  HttpSocket();

  /** Destructor.
   */
  virtual ~HttpSocket();

 private:
  // Disabled
  HttpSocket(const HttpSocket &);
  HttpSocket &operator=(const HttpSocket &);
};

}  // namespace ES

#endif
