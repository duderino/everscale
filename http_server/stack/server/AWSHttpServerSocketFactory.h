/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_SERVER_SOCKET_FACTORY_H
#define AWS_HTTP_SERVER_SOCKET_FACTORY_H

#ifndef AWS_HTTP_SERVER_SOCKET_H
#include <AWSHttpServerSocket.h>
#endif

#ifndef ESF_EMBEDDED_LIST_H
#include <ESFEmbeddedList.h>
#endif

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

#ifndef ESF_DISCARD_ALLOCATOR_H
#include <ESFDiscardAllocator.h>
#endif

#ifndef AWS_HTTP_SERVER_COUNTERS_H
#include <AWSHttpServerCounters.h>
#endif

#ifndef AWS_HTTP_SERVER_HANDLER_H
#include <AWSHttpServerHandler.h>
#endif

/** A factory that creates and reuses AWSHttpServerSockets
 */
class AWSHttpServerSocketFactory
{
public:

    AWSHttpServerSocketFactory(AWSHttpServerCounters *counters,
                               ESFLogger *logger);

    virtual ~AWSHttpServerSocketFactory();

    ESFError initialize();

    void destroy();

    AWSHttpServerSocket *create(AWSHttpServerHandler *handler, ESFTCPSocket::AcceptData *acceptData);

    void release(AWSHttpServerSocket *socket);

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return Memory for the new object or NULL if the memory allocation failed.
     */
    inline void *operator new(size_t size, ESFAllocator *allocator)
    {
        return allocator->allocate( size );
    }

private:
    // Disabled
    AWSHttpServerSocketFactory(const AWSHttpServerSocketFactory &);
    AWSHttpServerSocketFactory &operator=(const AWSHttpServerSocketFactory &);

    class CleanupHandler : public ESFCleanupHandler
    {
    public:
        /** Constructor
         */
        CleanupHandler(AWSHttpServerSocketFactory *factory);

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

        AWSHttpServerSocketFactory *_factory;
    };

    ESFLogger *_logger;
    AWSHttpServerCounters *_counters;
    ESFDiscardAllocator _allocator;
    ESFEmbeddedList _embeddedList;
    ESFMutex _mutex;
    CleanupHandler _cleanupHandler;
};

#endif /* ! AWS_HTTP_SERVER_SOCKET_FACTORY_H */
