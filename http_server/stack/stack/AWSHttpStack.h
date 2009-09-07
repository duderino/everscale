/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_STACK_H
#define AWS_HTTP_STACK_H

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_DISCARD_ALLOCATOR_H
#include <ESFDiscardAllocator.h>
#endif

#ifndef ESF_EPOLL_MULTIPLEXER_FACTORY_H
#include <ESFEpollMultiplexerFactory.h>
#endif

#ifndef ESF_SOCKET_MULTIPLEXER_DISPATCHER_H
#include <ESFSocketMultiplexerDispatcher.h>
#endif

#ifndef ESF_SHARED_ALLOCATOR_H
#include <ESFSharedAllocator.h>
#endif

#ifndef ESF_ALLOCATOR_CLEANUP_HANDLER_H
#include <ESFAllocatorCleanupHandler.h>
#endif

#ifndef AWS_HTTP_LISTENING_SOCKET_H
#include <AWSHttpListeningSocket.h>
#endif

#ifndef AWS_HTTP_SERVER_SOCKET_H
#include <AWSHttpServerSocket.h>
#endif

#ifndef AWS_HTTP_SERVER_HANDLER_H
#include <AWSHttpServerHandler.h>
#endif

#ifndef AWS_HTTP_SERVER_SOCKET_FACTORY_H
#include <AWSHttpServerSocketFactory.h>
#endif

#ifndef AWS_HTTP_CLIENT_SOCKET_FACTORY_H
#include <AWSHttpClientSocketFactory.h>
#endif

#ifndef AWS_HTTP_CONNECTION_POOL_H
#include <AWSHttpConnectionPool.h>
#endif

#ifndef AWS_HTTP_CLIENT_TRANSACTION_FACTORY_H
#include <AWSHttpClientTransactionFactory.h>
#endif

#ifndef AWS_HTTP_SERVER_HANDLER_H
#include <AWSHttpServerHandler.h>
#endif

#ifndef AWS_HTTP_CLIENT_HANDLER_H
#include <AWSHttpClientHandler.h>
#endif

#ifndef AWS_HTTP_CONNECTION_POOL_H
#include <AWSHttpConnectionPool.h>
#endif

#ifndef AWS_HTTP_RESOLVER_H
#include <AWSHttpResolver.h>
#endif

class AWSHttpStack : public AWSHttpConnectionPool
{
public:

    AWSHttpStack(AWSHttpServerHandler *serverHandler,
                 AWSHttpResolver *resolver,
                 int port,
                 int threads,
                 ESFLogger *logger);

    AWSHttpStack(AWSHttpResolver *resolver, int threads, ESFLogger *logger);

    virtual ~AWSHttpStack();

    ESFError initialize();

    ESFError start();

    ESFError stop();

    void destroy();

    /**
     * Create a new client transaction
     *
     * @param handler The handler
     * @return a new client transaction if successful, null otherwise
     */
    virtual AWSHttpClientTransaction *createClientTransaction(AWSHttpClientHandler *clientHandler);

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
    virtual ESFError executeClientTransaction(AWSHttpClientTransaction *transaction);

    /**
     * Cleanup the client transaction.  Note that this will not free any app-specific
     * context.  Call this only if executeClientTransaction doesn't return ESF_SUCCESS
     *
     * @param transaction The transaction to cleanup.
     */
    virtual void destroyClientTransaction(AWSHttpClientTransaction *transaction);

    inline const AWSPerformanceCounter *getClientSuccessCounter() const
    {
        return &_clientSuccessCounter;
    }

    inline const AWSPerformanceCounter *getClientFailureCounter() const
    {
        return &_clientFailureCounter;
    }

    inline AWSHttpServerCounters *getServerCounters()
    {
        return &_serverCounters;
    }

private:
    // disabled
    AWSHttpStack(const AWSHttpStack &);
    void operator=(const AWSHttpStack &);

    typedef enum
    {
        AWS_HTTP_STACK_IS_INITIALIZED = 0,
        AWS_HTTP_STACK_IS_STARTED = 1,
        AWS_HTTP_STACK_IS_STOPPED = 2,
        AWS_HTTP_STACK_IS_DESTROYED = 3
    } AWSHttpStackState;

    int _port;
    int _threads;
    volatile AWSHttpStackState _state;
    AWSHttpResolver *_resolver;
    ESFLogger *_logger;
    AWSHttpServerHandler *_serverHandler;
    ESFDiscardAllocator _discardAllocator;
    ESFSharedAllocator _rootAllocator;
    ESFAllocatorCleanupHandler _rootAllocatorCleanupHandler;
    ESFEpollMultiplexerFactory _epollFactory;
    ESFListeningTCPSocket _listeningSocket;
    AWSHttpServerCounters _serverCounters;
    AWSHttpServerSocketFactory _serverSocketFactory;
    AWSPerformanceCounter _clientSuccessCounter;
    AWSPerformanceCounter _clientFailureCounter;
    AWSHttpClientSocketFactory _clientSocketFactory;
    AWSHttpClientTransactionFactory _clientTransactionFactory;
    ESFSocketMultiplexerDispatcher _dispatcher;
};

#endif
