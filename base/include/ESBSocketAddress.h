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
    TLS = 3
    /**< TLS transport. */
  } TransportType;

  /** Default constructor.  Suitable for creating an uninitialized address.
   *
   */
  SocketAddress();

  /** Construct a new SocketAddress given a dotted-decimal IP address
   *  (e.g., "192.168.0.0"), port, and transport type.
   *
   *  If the address is known only by hostname, the default constructor
   *  should be used to construct the instance and the resolve method should
   *  be used to set the instance's values.  It's done this way because the
   *  hostname resolution can fail and we don't want to throw exceptions.
   *
   *  @param dottedIp The address's dotted IP address.
   *  @param port The address's port number.  Host byte order.
   *  @param transport The address's transport type.
   *  @see resolve.
   */
  SocketAddress(const char *dottedIp, UInt16 port, TransportType transport);

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

  /** Less than operator
   *
   * @param address The lhs instance
   * @return true if this instance is less than address
   */
  bool operator<(const SocketAddress &address) const;

  /** @todo Add setAddress member - wrap inet_pton, etc. */

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

  /** Get the IP address of the SocketAddress.
   *
   *  @param address A character array large enough to hold the ip address
   *      as a dotted-decimal string (e.g., "255.255.255.255").  16 chars
   *      is long enough to hold a dotted-decimal string for IPv4 with
   *      the terminating NULL character.
   *  @param size The size of the character array.
   */
  void presentationAddress(char *address, int size) const;

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

  /** Determine whether the socket address is valid (i.e., ip address and
   *  port number fall within valid range, transport type is set to
   *  one of the values in the TransportType enum except NONE).
   *
   *  @return true if the address is valid, false otherwise.
   */
  bool isValid();

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 private:
  Address _address;
  TransportType _transport;
  UInt8 _magic;
};

}  // namespace ESB

#endif
