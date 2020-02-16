/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_ECHO_CLIENT_CONTEXT_H
#define AWS_HTTP_ECHO_CLIENT_CONTEXT_H

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

class AWSHttpEchoClientContext {
 public:
  AWSHttpEchoClientContext(unsigned int remainingIterations);

  virtual ~AWSHttpEchoClientContext();

  inline unsigned int getBytesSent() { return _bytesSent; }

  inline void setBytesSent(unsigned int bytesSent) { _bytesSent = bytesSent; }

  inline unsigned int getRemainingIterations() { return _iterations; }

  inline void setRemainingIterations(unsigned int iterations) {
    _iterations = iterations;
  }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  // Disabled
  AWSHttpEchoClientContext(const AWSHttpEchoClientContext &context);
  void operator=(const AWSHttpEchoClientContext &context);

  unsigned int _bytesSent;
  unsigned int _iterations;
};

#endif
