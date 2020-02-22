/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_RESOLVER_H
#define AWS_HTTP_RESOLVER_H

#ifndef ESF_SOCKET_ADDRESS_H
#include <ESFSocketAddress.h>
#endif

#ifndef AWS_HTTP_REQUEST_H
#include <AWSHttpRequest.h>
#endif

class AWSHttpResolver {
 public:
  AWSHttpResolver();

  virtual ~AWSHttpResolver();

  virtual ESFError resolve(const AWSHttpRequest *request,
                           ESFSocketAddress *address) = 0;

 private:
  // Disabled
  AWSHttpResolver(const AWSHttpResolver &);
  void operator=(const AWSHttpResolver &);
};

#endif
