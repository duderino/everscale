/** @file ESFSocketAddress.h
 *  @brief A transport address object
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_SOCKET_ADDRESS_H
#define ESF_SOCKET_ADDRESS_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

#ifndef ESF_ERROR_H
#include <ESFError.h>
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

/** ESFSocketAddresses identify socket endpoints with their IP address,
 *  port, and transport type.
 *
 *  @ingroup network
 */
class ESFSocketAddress {
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
  ESFSocketAddress();

  /** Construct a new ESFSocketAddress given a dotted-decimal IP address
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
  ESFSocketAddress(const char *dottedIp, ESFUInt16 port,
                   TransportType transport);

  /** Copy constructor.
   *
   *  @param address The ESFSocketAddress instance to copy.
   */
  ESFSocketAddress(const ESFSocketAddress &address);

  /** Assignment operator.
   *
   *  @param address The ESFSocketAddress instance to copy.
   */
  ESFSocketAddress &operator=(const ESFSocketAddress &address);

  /** @todo Add setAddress member - wrap inet_pton, etc. */

  /** Get the address of ESFSocketAddress for use by the platform's socket
   *  API.  On most UNIXes this is actually the sockaddr_in structure.
   *
   *  @return the address
   */
  Address *getAddress();

  /** Get the IP address of the ESFSocketAddress.
   *
   *  @param address A character array large enough to hold the ip address
   *      as a dotted-decimal string (e.g., "255.255.255.255").  16 chars
   *      is long enough to hold a dotted-decimal string for IPv4 with
   *      the terminating NULL character.
   *  @param size The size of the character array.
   */
  void getIPAddress(char *address, int size) const;

  /** Get the port of the ESFSocketAddress.
   *
   *  @return the port.  Host byte order
   */
  ESFUInt16 getPort() const;

  /** Set the port of the ESFSocketAddress.
   *
   *  @param port the port.  Host byte order.
   */
  void setPort(ESFUInt16 port);

  /** Get the transport type of the ESFSocketAddress.
   *
   *  @return the transport type.
   */
  TransportType getTransport() const;

  /** Set the transport of the ESFSocketAddress.
   *
   *  @param transport The transport type.
   */
  void setTransport(TransportType transport);

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
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  Address _address;
  TransportType _transport;
  ESFUInt8 _magic;
};

#endif /* ! ESF_SOCKET_ADDRESS_H */
