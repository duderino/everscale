/* Copyright (c) 2011 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_CLIENT_SOCKET_FACTORY_H
#define AWS_HTTP_CLIENT_SOCKET_FACTORY_H

#ifndef ESF_EMBEDDED_LIST_H
#include <ESFEmbeddedList.h>
#endif

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

#ifndef ESF_DISCARD_ALLOCATOR_H
#include <ESFDiscardAllocator.h>
#endif

#ifndef ESF_CLEANUP_HANDLER_H
#include <ESFCleanupHandler.h>
#endif

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

#ifndef ESF_SOCKET_ADDRESS_H
#include <ESFSocketAddress.h>
#endif

#ifndef ESF_MAP_H
#include <ESFMap.h>
#endif

#ifndef AWS_HTTP_CLIENT_SOCKET_H
#include <AWSHttpClientSocket.h>
#endif

#ifndef AWS_PERFORMANCE_COUNTER_H
#include <AWSPerformanceCounter.h>
#endif

#ifndef AWS_HTTP_CLIENT_TRANSACTION_H
#include <AWSHttpClientTransaction.h>
#endif

#ifndef AWS_HTTP_CONNECTION_POOL_H
#include <AWSHttpConnectionPool.h>
#endif

#ifndef AWS_HTTP_CLIENT_COUNTERS_H
#include <AWSHttpClientCounters.h>
#endif

/** A factory that creates and reuses AWSHttpClientSockets
 */
class AWSHttpClientSocketFactory {
public:

    /** Constructor
     *
     * @param logger A logger
     */
    AWSHttpClientSocketFactory(AWSHttpClientCounters *clientCounters, ESFLogger *logger);

    /** Destructor.
     */
    virtual ~AWSHttpClientSocketFactory();

    ESFError initialize();

    void destroy();

    AWSHttpClientSocket *create(AWSHttpConnectionPool *pool, AWSHttpClientTransaction *transaction);

    void release(AWSHttpClientSocket *socket);

private:
    // Disabled
    AWSHttpClientSocketFactory(const AWSHttpClientSocketFactory &);
    AWSHttpClientSocketFactory &operator=(const AWSHttpClientSocketFactory &);

    class CleanupHandler: public ESFCleanupHandler {
    public:
        /** Constructor
         */
        CleanupHandler(AWSHttpClientSocketFactory *factory);

        /** Destructor
         */
        virtual ~CleanupHandler();

        /** Destroy an object
         *
         * @param object The object to destroy
         */
        virtual void destroy(ESFObject *object);

    private:
        // Disabled
        CleanupHandler(const CleanupHandler &);
        void operator=(const CleanupHandler &);

        AWSHttpClientSocketFactory *_factory;
    };

    ESFLogger *_logger;
    AWSHttpClientCounters *_clientCounters;
    ESFDiscardAllocator _allocator;
    ESFMap _map;
    ESFEmbeddedList _embeddedList;
    ESFMutex _mutex;
    CleanupHandler _cleanupHandler;
};

#endif
