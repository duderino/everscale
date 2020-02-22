/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_SERVER_TRANSACTION_H
#include <AWSHttpServerTransaction.h>
#endif

AWSHttpServerTransaction::AWSHttpServerTransaction()
    : AWSHttpTransaction(0),
      _parser(getWorkingBuffer(), &_allocator),
      _formatter() {}

AWSHttpServerTransaction::~AWSHttpServerTransaction() {}

void AWSHttpServerTransaction::reset() {
  AWSHttpTransaction::reset();
  _parser.reset();
  _formatter.reset();
}
