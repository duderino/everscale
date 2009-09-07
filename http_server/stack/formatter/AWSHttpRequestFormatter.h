/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_REQUEST_FORMATTER_H
#define AWS_HTTP_REQUEST_FORMATTER_H

#ifndef ESF_BUFFER_H
#include <ESFBuffer.h>
#endif

#ifndef AWS_HTTP_REQUEST_H
#include <AWSHttpRequest.h>
#endif

#ifndef AWS_HTTP_REQUEST_URI_FORMATTER_H
#include <AWSHttpRequestUriFormatter.h>
#endif

#ifndef AWS_HTTP_MESSAGE_FORMATTER_H
#include <AWSHttpMessageFormatter.h>
#endif

/**
 * Formats a HTTP request as defined in RFC 2616 and RFC 2396
 */
class AWSHttpRequestFormatter : public AWSHttpMessageFormatter
{
public:

    /** Create a new request formatter
     */
    AWSHttpRequestFormatter();

    virtual ~AWSHttpRequestFormatter();

    /**
     * Reset the formatter
     */
    virtual void reset();

protected:

    // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
    virtual ESFError formatStartLine(ESFBuffer *outputBuffer, const AWSHttpMessage *message);

private:

    // Disabled
    AWSHttpRequestFormatter(const AWSHttpRequestFormatter &formatter);
    void operator=(const AWSHttpRequestFormatter &formatter);

    // Method                = "OPTIONS"                ; Section 9.2
    //                       | "GET"                    ; Section 9.3
    //                       | "HEAD"                   ; Section 9.4
    //                       | "POST"                   ; Section 9.5
    //                       | "PUT"                    ; Section 9.6
    //                       | "DELETE"                 ; Section 9.7
    //                       | "TRACE"                  ; Section 9.8
    //                       | "CONNECT"                ; Section 9.9
    //                       | extension-method
    // extension-method = token
    ESFError formatMethod(ESFBuffer *outputBuffer, const AWSHttpRequest *request);

    int _requestState;
    AWSHttpRequestUriFormatter _requestUriFormatter;
};

#endif
