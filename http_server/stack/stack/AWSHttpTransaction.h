/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_TRANSACTION_H
#define AWS_HTTP_TRANSACTION_H

#ifndef ESF_SOCKET_ADDRESS_H
#include <ESFSocketAddress.h>
#endif

#ifndef ESF_DISCARD_ALLOCATOR_H
#include <ESFDiscardAllocator.h>
#endif

#ifndef ESF_EMBEDDED_LIST_ELEMENT_H
#include <ESFEmbeddedListElement.h>
#endif

#ifndef ESF_BUFFER_H
#include <ESFBuffer.h>
#endif

#ifndef AWS_HTTP_UTIL_H
#include <AWSHttpUtil.h>
#endif

#ifndef AWS_HTTP_HEADER_H
#include <AWSHttpHeader.h>
#endif

#ifndef AWS_HTTP_REQUEST_H
#include <AWSHttpRequest.h>
#endif

#ifndef AWS_HTTP_RESPONSE_H
#include <AWSHttpResponse.h>
#endif

#ifndef AWS_PERFORMANCE_COUNTER_H
#include <AWSPerformanceCounter.h>
#endif

#ifndef AWS_HTTP_IO_BUFFER_SIZE
#define AWS_HTTP_IO_BUFFER_SIZE 4096
#endif

#ifndef AWS_HTTP_WORKING_BUFFER_SIZE
#define AWS_HTTP_WORKING_BUFFER_SIZE 2048
#endif

// TODO buffers should not be exposed here.  Move to a subclass that is not
// visible to the client and server handlers.
class AWSHttpTransaction : public ESFEmbeddedListElement
{
public:

    AWSHttpTransaction(ESFCleanupHandler *cleanupHandler);

    AWSHttpTransaction(ESFSocketAddress *peerAddress, ESFCleanupHandler *cleanupHandler);

    virtual ~AWSHttpTransaction();

    inline const ESFSocketAddress *getPeerAddress() const
    {
        return &_peerAddress;
    }

    inline ESFSocketAddress *getPeerAddress()
    {
        return &_peerAddress;
    }

    inline void setPeerAddress(const ESFSocketAddress *peerAddress)
    {
        if (peerAddress)
        {
            _peerAddress = *peerAddress;
        }
    }

    virtual void reset();

    inline unsigned char *duplicate(unsigned char *value)
    {
        return AWSHttpUtil::Duplicate(&_allocator, value);
    }

    AWSHttpHeader *createHeader(unsigned const char *name, unsigned const char *value);

    inline ESFAllocator *getAllocator()
    {
        return &_allocator;
    }

    inline const AWSHttpRequest *getRequest() const
    {
        return &_request;
    }

    inline AWSHttpRequest *getRequest()
    {
        return &_request;
    }

    inline const AWSHttpResponse *getResponse() const
    {
        return &_response;
    }

    inline AWSHttpResponse *getResponse()
    {
        return &_response;
    }

    inline void setApplicationContext(void *appContext)
    {
        _appContext = appContext;
    }

    inline void *getApplicationContext()
    {
        return _appContext;
    }

    inline const void *getApplicationContext() const
    {
        return _appContext;
    }

    inline ESFBuffer *getIOBuffer()
    {
        return &_ioBuffer;
    }

    inline const ESFBuffer *getIOBuffer() const
    {
        return &_ioBuffer;
    }

    inline ESFBuffer *getWorkingBuffer()
    {
        return &_workingBuffer;
    }

    inline const ESFBuffer *getWorkingBuffer() const
    {
        return &_workingBuffer;
    }

    /** Return an optional handler that can destroy the element.
     *
     * @return A handler to destroy the element or NULL if the element should not be destroyed.
     */
    virtual ESFCleanupHandler *getCleanupHandler();

    inline void setStartTime()
    {
        AWSPerformanceCounter::GetTime(&_start);
    }

    inline const struct timeval *getStartTime() const
    {
        return &_start;
    }

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return Memory for the new object or NULL if the memory allocation failed.
     */
    inline void *operator new(size_t size, ESFAllocator *allocator)
    {
        return allocator->allocate(size);
    }

protected:

    ESFDiscardAllocator _allocator;

private:

    // Disabled
    AWSHttpTransaction(const AWSHttpTransaction &transaction);
    void operator=(const AWSHttpTransaction &transaction);

    void *_appContext;
    ESFCleanupHandler *_cleanupHandler;
    struct timeval _start;
    ESFSocketAddress _peerAddress;
    AWSHttpRequest _request;
    AWSHttpResponse _response;
    ESFBuffer _ioBuffer;
    ESFBuffer _workingBuffer;
    unsigned char _ioBufferStorage[AWS_HTTP_IO_BUFFER_SIZE];
    unsigned char _workingBufferStorage[AWS_HTTP_WORKING_BUFFER_SIZE];
};

#endif

