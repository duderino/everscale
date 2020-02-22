/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_DEFAULT_RESOLVER_H
#define AWS_HTTP_DEFAULT_RESOLVER_H

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

#ifndef AWS_HTTP_RESOLVER_H
#include <AWSHttpResolver.h>
#endif

class AWSHttpDefaultResolver : public AWSHttpResolver {
 public:
  AWSHttpDefaultResolver(ESFLogger *logger);

  virtual ~AWSHttpDefaultResolver();

  virtual ESFError resolve(const AWSHttpRequest *request,
                           ESFSocketAddress *address);

 private:
  // Disabled
  AWSHttpDefaultResolver(const AWSHttpDefaultResolver &);
  void operator=(const AWSHttpDefaultResolver &);

  ESFLogger *_logger;
};

#endif
