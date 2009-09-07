/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_CLIENT_TRANSACTION_FACTORY_H
#include <AWSHttpClientTransactionFactory.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_FIXED_ALLOCATOR_H
#include <ESFFixedAllocator.h>
#endif

#ifndef ESF_NULL_LOGGER_H
#include <ESFNullLogger.h>
#endif

AWSHttpClientTransactionFactory::AWSHttpClientTransactionFactory() :
    _allocator(),
    _embeddedList(),
    _mutex(),
    _cleanupHandler(this)
{
}

ESFError AWSHttpClientTransactionFactory::initialize()
{
    return _allocator.initialize(ESF_WORD_ALIGN(sizeof(AWSHttpClientTransaction)) * 1000,
                                 ESFSystemAllocator::GetInstance());
}

void AWSHttpClientTransactionFactory::destroy()
{
    AWSHttpClientTransaction *transaction = (AWSHttpClientTransaction *) _embeddedList.removeFirst();

    while (transaction)
    {
        transaction->~AWSHttpClientTransaction();

        transaction = (AWSHttpClientTransaction *) _embeddedList.removeFirst();
    }

    _allocator.destroy();
}

AWSHttpClientTransactionFactory::~AWSHttpClientTransactionFactory()
{
}

AWSHttpClientTransaction *AWSHttpClientTransactionFactory::create(AWSHttpClientHandler *clientHandler)
{
    AWSHttpClientTransaction *transaction = 0;

    _mutex.writeAcquire();

    transaction = (AWSHttpClientTransaction *) _embeddedList.removeLast();

    _mutex.writeRelease();

    if (0 == transaction)
    {
        transaction = new(&_allocator) AWSHttpClientTransaction(clientHandler, &_cleanupHandler);

        if (0 == transaction)
        {
            return 0;
        }
    }

    transaction->setHandler(clientHandler);

    return transaction;
}

void AWSHttpClientTransactionFactory::release(AWSHttpClientTransaction *transaction)
{
    if (! transaction)
    {
        return;
    }

    transaction->reset();

    _mutex.writeAcquire();

    _embeddedList.addLast(transaction);

    _mutex.writeRelease();
}

AWSHttpClientTransactionFactory::CleanupHandler::CleanupHandler(AWSHttpClientTransactionFactory *factory) :
    ESFCleanupHandler(),
    _factory(factory)
{
}

AWSHttpClientTransactionFactory::CleanupHandler::~CleanupHandler()
{
}

void AWSHttpClientTransactionFactory::CleanupHandler::destroy(ESFObject *object)
{
    _factory->release((AWSHttpClientTransaction *) object);
}

