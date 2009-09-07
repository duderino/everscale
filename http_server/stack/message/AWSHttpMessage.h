/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_MESSAGE_H
#define AWS_HTTP_MESSAGE_H

#ifndef ESF_EMBEDDED_LIST_H
#include <ESFEmbeddedList.h>
#endif

#ifndef AWS_HTTP_HEADER_H
#include <AWSHttpHeader.h>
#endif

#define AWS_HTTP_MESSAGE_HAS_BODY (1 << 0)
#define AWS_HTTP_MESSAGE_REUSE_CONNECTION (1 << 1)
#define AWS_HTTP_MESSAGE_SEND_100_CONTINUE (1 << 2)

/**
 * A HTTP message as defined in RFC 2616 and RFC 2396
 */
class AWSHttpMessage
{
public:
    AWSHttpMessage();

    virtual ~AWSHttpMessage();

    inline ESFEmbeddedList *getHeaders()
    {
        return &_headers;
    }

    inline const ESFEmbeddedList *getHeaders() const
    {
        return &_headers;
    }

    const AWSHttpHeader *getHeader(unsigned const char *fieldName) const;

    ESFError addHeader(unsigned const char *fieldName,
                       unsigned const char *fieldValue,
                       ESFAllocator *allocator);

    ESFError addHeader(ESFAllocator *allocator,
                       unsigned const char *fieldName,
                       unsigned const char *fieldValueFormat,
                       ...);

    /**
     * Get the HTTP version.
     *
     * @return The HTTP version.  110 is HTTP/1.1, 100 is HTTP/1.0, etc.
     */
    inline int getHttpVersion() const
    {
        return _version;
    }

    /**
     * Set the HTTP version.
     *
     * @param vesion The HTTP version.  110 is HTTP/1.1, 100 is HTTP/1.0, etc.
     */
    inline void setHttpVersion(int version)
    {
        _version = version;
    }

    inline void reset()
    {
        _version = 110;
        _headers.clear(false);  // don't call cleanup handlers
    }

    inline void setHasBody(bool hasBody)
    {
        if (hasBody)
        {
            _flags |= AWS_HTTP_MESSAGE_HAS_BODY;
        }
        else
        {
            _flags &= ~AWS_HTTP_MESSAGE_HAS_BODY;
        }
    }

    inline bool hasBody() const
    {
        return _flags & AWS_HTTP_MESSAGE_HAS_BODY;
    }

    inline void setReuseConnection(bool reuseConnection)
    {
        if (reuseConnection)
        {
            _flags |= AWS_HTTP_MESSAGE_REUSE_CONNECTION;
        }
        else
        {
            _flags &= ~AWS_HTTP_MESSAGE_REUSE_CONNECTION;
        }
    }

    inline bool getReuseConnection() const
    {
        return _flags & AWS_HTTP_MESSAGE_REUSE_CONNECTION;
    }

    inline void setSend100Continue(bool send100Continue)
    {
        if (send100Continue)
        {
            _flags |= AWS_HTTP_MESSAGE_SEND_100_CONTINUE;
        }
        else
        {
            _flags &= ~AWS_HTTP_MESSAGE_SEND_100_CONTINUE;
        }
    }

    inline bool getSend100Continue()
    {
        return _flags & AWS_HTTP_MESSAGE_SEND_100_CONTINUE;
    }

private:

    // Disabled
    AWSHttpMessage(const AWSHttpMessage &);
    void operator=(const AWSHttpMessage &);

    int _flags;
    int _version;       // 110 is 1.1, 100 is 1.0, etc.
    ESFEmbeddedList _headers;
};

#endif
