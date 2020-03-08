#ifndef ESB_SYSTEM_DNS_CLIENT_H
#include <ESBSystemDnsClient.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#include <netdb.h>

namespace ESB {

SystemDnsClient::SystemDnsClient() : DnsClient() {}

SystemDnsClient::~SystemDnsClient() {}

Error SystemDnsClient::resolve(SocketAddress *address,
                               const unsigned char *hostname, UInt16 port,
                               bool isSecure) {
  if (0 == hostname || 0 == address) {
    return ESB_NULL_POINTER;
  }

  /* Linux:
   *
   * Glibc2  also  has  reentrant versions gethostbyname_r() and gethostby-
   * name2_r().  These return 0 on success and nonzero on error. The result
   * of  the  call is now stored in the struct with address ret.  After the
   * call, *result will be NULL on error or point to the result on success.
   * Auxiliary  data is stored in the buffer buf of length buflen.  (If the
   * buffer is too small, these functions will return ERANGE.)   No  global
   * variable  h_errno  is modified, but the address of a variable in which
   * to store error numbers is passed in h_errnop.
   *
   * Stevens:
   *
   * Current implementations of gethostbyname can return up to 35 alias
   * pointers, 35 address pointers, and internally use an 8192-byte buffer to
   * hold the alias names and addresses.  So a buffer size of 8192 bytes
   * should be adequate
   */

  char buffer[8192];
  struct hostent hostEntry;
  int hostErrno = 0;
  struct hostent *result = 0;

  memset(&hostEntry, 0, sizeof(hostEntry));

  if (0 != gethostbyname_r((const char *)hostname, &hostEntry, buffer,
                           sizeof(buffer), &result, &hostErrno)) {
    switch (hostErrno) {
      case HOST_NOT_FOUND:
      case NO_ADDRESS:
        ESB_LOG_ERRNO_DEBUG(hostErrno, "Cannot resolve hostname %s", hostname);
        return ESB_CANNOT_FIND;
      case TRY_AGAIN:
        ESB_LOG_ERRNO_DEBUG(hostErrno, "Temporary error resolving hostname %s", hostname);
        return ESB_AGAIN;
      case NO_RECOVERY:
      default:
        ESB_LOG_ERRNO_DEBUG(hostErrno, "Permanent error resolving hostname %s", hostname);
        return ESB_OTHER_ERROR;
    }
  }

  memset(address->getAddress(), 0, sizeof(SocketAddress::Address));

  memcpy(&address->getAddress()->sin_addr, hostEntry.h_addr_list[0],
         sizeof(address->getAddress()->sin_addr));
  address->getAddress()->sin_family = AF_INET;
  address->getAddress()->sin_port = htons(port);

  address->setTransport(isSecure ? SocketAddress::TLS : SocketAddress::TCP);

  return ESB_SUCCESS;
}

}  // namespace ESB
