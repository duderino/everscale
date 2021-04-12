#ifndef ESB_SOCKET_ADDRESS_H
#define ESB_SOCKET_ADDRESS_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

#ifndef ESB_STRING_H
#include <ESBString.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

namespace ESB {

/** SocketAddresses identify socket endpoints with their IP address,
 *  port, and transport type.
 *
 *  @ingroup network
 */
class SocketAddress {
 public:
#ifdef HAVE_STRUCT_SOCKADDR_IN
  typedef struct sockaddr_in Address;
#else
#error "sockaddr_in or equivalent is required"
#endif

  /** The transport type of the address.  IP Address and port number uniquely
   *  identify an endpoint only within transport types.
   */
  typedef enum {
    NONE = 0,
    UDP = 1, /**< UDP transport. */
    TCP = 2, /**< TCP transport. */
    TLS = 3  /**< TLS transport. */
  } TransportType;

  /** Default constructor.  Suitable for creating an uninitialized address.
   *
   */
  SocketAddress();

  /** Construct a new SocketAddress with an address in presentation format
   * (e.g., "192.168.0.0"), port, and transport type.
   *
   *  @param presentation The address in presentation format
   *  @param port The address's port number.  Host byte order.
   *  @param transport The address's transport type.
   *  @see resolve.
   */
  SocketAddress(const char *presentation, UInt16 port, TransportType transport);

  /** Copy constructor.
   *
   *  @param address The SocketAddress instance to copy.
   */
  SocketAddress(const SocketAddress &address);

  /** Assignment operator.
   *
   *  @param address The SocketAddress instance to copy.
   */
  SocketAddress &operator=(const SocketAddress &address);

  /**
   * Compare two socket addresses.
   *
   * @param address the other address to compare this to
   * @return 0 if equal, less than zero if this address is less than the other address, greater than zero if this
   * address is greater than the other address.
   */
  inline int compare(const SocketAddress &address) const {
    int result = _transport - address._transport;
    if (0 != result) {
      return result;
    }

    result = _address.sin_port - address._address.sin_port;
    if (0 != result) {
      return result;
    }

    // TODO support IPv6
    return _address.sin_addr.s_addr - address._address.sin_addr.s_addr;
  }

  /**
   * Get a one-way hash representation of this object.
   *
   * @return a hash code
   */
  inline ESB::UInt64 hash() const {
    // TODO not IPv6 safe, for IPv6 take the low order bits of the address
    ESB::UInt64 hash = _address.sin_addr.s_addr;
    hash |= (UInt64)_address.sin_port << 32;
    hash |= (UInt64)_transport << 48;
    return hash;
  }

  /** Get the address of SocketAddress for use by the platform's socket
   *  API.  On most UNIXes this is actually the sockaddr_in structure.
   *
   *  @return the address
   */
  const Address *primitiveAddress() const;

  /** Get the address of SocketAddress for use by the platform's socket
   *  API.  On most UNIXes this is actually the sockaddr_in structure.
   *
   *  @return the address
   */
  Address *primitiveAddress();

  void updatePrimitiveAddress(Address *address);

  /** Get the IP address of the SocketAddress in human readable format.
   *
   *  @param address A character array of at least ESB_IPV6_PRESENTATION_SIZE
   * bytes.
   *  @param size The size of the character array.
   */
  void presentationAddress(char *address, int size) const;

  /** Get the IP address+port of the SocketAddress in human readable format.
   *
   *  @param address A character array of at least ESB_LOG_ADDRESS_SIZE bytes
   *  @param size The size of the character array.
   *  @param fd an optional file descriptor to include in the log address
   *  @return bytes written to address excluding terminating NULL character
   */
  int logAddress(char *address, int size, int fd) const;

  /** Update the IP address with an address in presentation format (e.g.,
   * "192.168.0.0").
   *
   *  @param presentation The address in presentation format
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error setAddress(const char *presentation);

  /** Get the port of the SocketAddress.
   *
   *  @return the port.  Host byte order
   */
  UInt16 port() const;

  /** Set the port of the SocketAddress.
   *
   *  @param port the port.  Host byte order.
   */
  void setPort(UInt16 port);

  /** Get the transport type of the SocketAddress.
   *
   *  @return the transport type.
   */
  TransportType type() const;

  /** Set the transport of the SocketAddress.
   *
   *  @param transport The transport type.
   */
  void setType(TransportType transport);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  Address _address;
  TransportType _transport;
};

}  // namespace ESB

#endif
