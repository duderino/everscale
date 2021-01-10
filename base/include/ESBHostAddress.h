#ifndef ESB_HOST_ADDRESS_H
#define ESB_HOST_ADDRESS_H

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_STRING_H
#include <ESBString.h>
#endif

namespace ESB {

/** HostAddresses add hostnames/fqdns to SocketAddresses
 *
 *  @ingroup network
 */
class HostAddress : public SocketAddress {
 public:
  /**
   * Default constructor.  Suitable for creating an uninitialized address.
   */
  HostAddress();

  /** Construct a new HostAddress with an address in presentation format
   * (e.g., "192.168.0.0"), port, and transport type.
   *
   *  @param host The fqdn or hostname associated with the address
   *  @param presentation The address in presentation format
   *  @param port The address's port number.  Host byte order.
   *  @param transport The address's transport type.
   */
  HostAddress(const char *host, const char *presentation, UInt16 port, TransportType transport);

  /**
   * Construct a new HostAddress from a hostname and a SocketAddress
   *
   * @param host The hostname
   * @param address The SocketAddress
   */
  HostAddress(const char *host, SocketAddress &address);

  /** Copy constructor.
   *
   *  @param address The HostAddress instance to copy.
   */
  HostAddress(const HostAddress &address);

  /** Assignment operator.
   *
   *  @param address The HostAddress instance to copy.
   */
  HostAddress &operator=(const HostAddress &address);

  virtual const char *host() const;

  inline void setHost(const char *host) {
    strncpy(_host, host, sizeof(_host));
    _host[sizeof(_host) - 1] = 0;
  }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  char _host[ESB_MAX_HOSTNAME + 1];
};

}  // namespace ESB

#endif
