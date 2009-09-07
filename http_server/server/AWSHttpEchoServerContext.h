/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_ECHO_SERVER_CONTEXT_H
#define AWS_HTTP_ECHO_SERVER_CONTEXT_H

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

class AWSHttpEchoServerContext
{
public:

    AWSHttpEchoServerContext();

    virtual ~AWSHttpEchoServerContext();

    inline unsigned int getBytesSent()
    {
        return _bytesSent;
    }

    inline void addBytesSent(unsigned int bytesSent)
    {
        _bytesSent += bytesSent;
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

private:
    // Disabled
    AWSHttpEchoServerContext(const AWSHttpEchoServerContext &state);
    void operator=(const AWSHttpEchoServerContext &state);

    unsigned int _bytesSent;
};

#endif






