/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_TRANSACTION_H
#include <AWSHttpTransaction.h>
#endif

#ifndef AWS_HTTP_UTIL_H
#include <AWSHttpUtil.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef AWS_HTTP_TRANSACTION_ALLOCATOR_CHUNK_SIZE
#define AWS_HTTP_TRANSACTION_ALLOCATOR_CHUNK_SIZE 2000
#endif

AWSHttpTransaction::AWSHttpTransaction(ESFCleanupHandler *cleanupHandler) :
    _allocator(AWS_HTTP_TRANSACTION_ALLOCATOR_CHUNK_SIZE, ESFSystemAllocator::GetInstance()),
    _appContext(0),
    _cleanupHandler(cleanupHandler),
    _peerAddress(),
    _request(),
    _response(),
    _ioBuffer(_ioBufferStorage, sizeof(_ioBufferStorage)),
    _workingBuffer(_workingBufferStorage, sizeof(_workingBufferStorage))
{
    memset(&_start, 0, sizeof(_start));
}

AWSHttpTransaction::AWSHttpTransaction(ESFSocketAddress *peerAddress, ESFCleanupHandler *cleanupHandler) :
    _allocator(AWS_HTTP_TRANSACTION_ALLOCATOR_CHUNK_SIZE, ESFSystemAllocator::GetInstance()),
    _appContext(0),
    _cleanupHandler(cleanupHandler),
    _peerAddress(*peerAddress),
    _request(),
    _response(),
    _ioBuffer(_ioBufferStorage, sizeof(_ioBufferStorage)),
    _workingBuffer(_workingBufferStorage, sizeof(_workingBufferStorage))
{
    memset(&_start, 0, sizeof(_start));
}

AWSHttpTransaction::~AWSHttpTransaction()
{
}

void AWSHttpTransaction::reset()
{
    _appContext = 0;

    ESFSocketAddress peerAddress;

    _peerAddress = peerAddress;

    _allocator.reset();
    _request.reset();
    _response.reset();
    //_ioBuffer.compact();
    _workingBuffer.clear();

    memset(&_start, 0, sizeof(_start));
}

AWSHttpHeader *AWSHttpTransaction::createHeader(unsigned const char *name, unsigned const char *value)
{
    unsigned char *fieldName = AWSHttpUtil::Duplicate(&_allocator, name);

    if (! fieldName)
    {
        return 0;
    }

    unsigned char *fieldValue = AWSHttpUtil::Duplicate(&_allocator, value);

    if (! fieldValue)
    {
        _allocator.deallocate(fieldName);

        return 0;
    }

    AWSHttpHeader *header = new(&_allocator) AWSHttpHeader(fieldName, fieldValue);

    if (! header)
    {
        _allocator.deallocate(fieldName);
        _allocator.deallocate(fieldValue);

        return 0;
    }

    return header;
}

ESFCleanupHandler *AWSHttpTransaction::getCleanupHandler()
{
    return _cleanupHandler;
}

