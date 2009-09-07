/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_SERVER_TRANSACTION_H
#define AWS_HTTP_SERVER_TRANSACTION_H

#ifndef AWS_HTTP_TRANSACTION_H
#include <AWSHttpTransaction.h>
#endif

#ifndef AWS_HTTP_SERVER_HANDLER_H
#include <AWSHttpServerHandler.h>
#endif

#ifndef AWS_HTTP_REQUEST_PARSER_H
#include <AWSHttpRequestParser.h>
#endif

#ifndef AWS_HTTP_RESPONSE_FORMATTER_H
#include <AWSHttpResponseFormatter.h>
#endif

class AWSHttpServerTransaction : public AWSHttpTransaction
{
public:

    AWSHttpServerTransaction();

    virtual ~AWSHttpServerTransaction();

    virtual void reset();

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

    inline AWSHttpRequestParser *getParser()
    {
        return &_parser;
    }

    inline const AWSHttpRequestParser *getParser() const
    {
        return &_parser;
    }

    inline AWSHttpResponseFormatter *getFormatter()
    {
        return &_formatter;
    }

    inline const AWSHttpResponseFormatter *getFormatter() const
    {
        return &_formatter;
    }

private:

    // Disabled
    AWSHttpServerTransaction(const AWSHttpServerTransaction &transaction);
    void operator=(const AWSHttpServerTransaction &transaction);

    AWSHttpRequestParser _parser;
    AWSHttpResponseFormatter _formatter;
};

#endif

