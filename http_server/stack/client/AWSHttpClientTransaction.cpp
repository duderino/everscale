/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_CLIENT_TRANSACTION_H
#include <AWSHttpClientTransaction.h>
#endif

AWSHttpClientTransaction::AWSHttpClientTransaction(AWSHttpClientHandler *clientHandler,
                                                   ESFCleanupHandler *cleanupHandler) :
    AWSHttpTransaction(cleanupHandler),
    _clientHandler(clientHandler),
    _parser(getWorkingBuffer(), &_allocator),
    _formatter()
{
}

AWSHttpClientTransaction::AWSHttpClientTransaction(AWSHttpClientHandler *clientHandler,
                                                   ESFSocketAddress *peerAddress,
                                                   ESFCleanupHandler *cleanupHandler) :
    AWSHttpTransaction(peerAddress, cleanupHandler),
    _clientHandler(clientHandler),
    _parser(getWorkingBuffer(), &_allocator),
    _formatter()
{
}

AWSHttpClientTransaction::~AWSHttpClientTransaction()
{
}

void AWSHttpClientTransaction::reset()
{
    AWSHttpTransaction::reset();

    _parser.reset();
    _formatter.reset();
    _clientHandler = 0;
}


