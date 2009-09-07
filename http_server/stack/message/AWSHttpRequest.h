/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_REQUEST_H
#define AWS_HTTP_REQUEST_H

#ifndef AWS_HTTP_REQUEST_URI_H
#include <AWSHttpRequestUri.h>
#endif

#ifndef AWS_HTTP_MESSAGE_H
#include <AWSHttpMessage.h>
#endif

/**
 * A HTTP Request as defined in RFC 2616 and RFC 2396
 */
class AWSHttpRequest : public AWSHttpMessage
{
public:

    AWSHttpRequest();

    virtual ~AWSHttpRequest();

    void reset();

    inline const unsigned char *getMethod() const
    {
        return _method;
    }

    inline void setMethod(const unsigned char *method)
    {
        _method = method;
    }

    inline AWSHttpRequestUri *getRequestUri()
    {
        return &_requestUri;
    }

    inline const AWSHttpRequestUri *getRequestUri() const
    {
        return &_requestUri;
    }

    /**
     * Parse the request uri and any Host header to determine the (unresolved)
     * address where this request should be sent to / was sent to.
     *
     * @param hostname A buffer to store the hostname
     * @param size The size of the hostname buffer
     * @param port A short to store the port
     * @param isSecure HTTPS vs. HTTP
     * @return ESF_SUCCESS if the request contained enough information to determine
     *   the peer address, another error code otherwise
     */
    ESFError parsePeerAddress(unsigned char *hostname,
                              int size,
                              ESFUInt16 *port,
                              bool *isSecure) const;

private:

    // Disabled
    AWSHttpRequest(const AWSHttpRequest &);
    void operator=(const AWSHttpRequest &);

    unsigned const char *_method;
    AWSHttpRequestUri _requestUri;
};

#endif
