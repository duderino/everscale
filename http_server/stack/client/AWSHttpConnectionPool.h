/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_CONNECTION_POOL_H
#define AWS_HTTP_CONNECTION_POOL_H

#ifndef AWS_HTTP_CLIENT_TRANSACTION_H
#include <AWSHttpClientTransaction.h>
#endif

#ifndef AWS_HTTP_CLIENT_HANDLER_H
#include <AWSHttpClientHandler.h>
#endif


class AWSHttpConnectionPool
{
public:

    AWSHttpConnectionPool();

    virtual ~AWSHttpConnectionPool();

    /**
     * Create a new client transaction
     *
     * @param handler The handler
     * @return a new client transaction if successful, null otherwise
     */
    virtual AWSHttpClientTransaction *createClientTransaction(AWSHttpClientHandler *clientHandler) = 0;

    /**
     * Execute the client transaction.  If this method returns ESF_SUCCESS, then the
     * transaction will be cleaned up automatically after it finishes.  If this method
     * returns anything else then the caller should clean it up with
     * destroyClientTransaction
     *
     * @param transaction The transaction
     * @return ESF_SUCCESS if the transaction was successfully started, another error
     *   code otherwise.  If error, cleanup the transaction with the destroyClientTransaction
     *   method.
     */
    virtual ESFError executeClientTransaction(AWSHttpClientTransaction *transaction) = 0;

    /**
     * Cleanup the client transaction.  Note that this will not free any app-specific
     * context.  Call this only if executeClientTransaction doesn't return ESF_SUCCESS
     *
     * @param transaction The transaction to cleanup.
     */
    virtual void destroyClientTransaction(AWSHttpClientTransaction *transaction) = 0;

private:
    // disabled
    AWSHttpConnectionPool(const AWSHttpConnectionPool &);
    void operator=(const AWSHttpConnectionPool &);

};

#endif
