/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_REQUEST_URI_PARSER_H
#define AWS_HTTP_REQUEST_URI_PARSER_H

#ifndef AWS_HTTP_REQUEST_URI_H
#include <AWSHttpRequestUri.h>
#endif

#ifndef ESF_DISCARD_ALLOCATOR_H
#include <ESFDiscardAllocator.h>
#endif

#ifndef ESF_BUFFER_H
#include <ESFBuffer.h>
#endif

/**
 * Parses a HTTP Request-URI as defined in RFC 2616 and RFC 2396
 *
 * TODO handle username:password@ in http(s) urls
 * TODO add class/functions to deep parse abs_path (and decode/encode path segements)
 * TODO add class/functions to deep parse query string (and decode/encode query args)
 */
class AWSHttpRequestUriParser
{
public:

    /** Create a new uri parser
     *
     * @param workingBuffer Temporary storage for parsing
     * @param allocator The discard allocator to use for allocating internal strings.
     */
    AWSHttpRequestUriParser(ESFBuffer *workingBuffer, ESFDiscardAllocator *allocator);

    virtual ~AWSHttpRequestUriParser();

    /**
     * Reset the parser
     */
    void reset();

    /**
     * Incrementally parse a Request-URI.
     *
     * Request-URI   = "*" | absoluteURI | abs_path [ "?" query ] | authority
     *
     * Caller should:
     * (1) Fill buffer, closing any idle sockets, enforcing any size limits.
     * (2) Parse buffer with this method. If ESF_AGAIN is returned, compact
     *     buffer.  If error or ESF_SUCCESS returned, break.
     * (3) If no space in buffer, parser is jammed, break and close socket.
     * (4) Otherwise goto (1)
     *
     * @param inputBuffer The buffer to parse
     * @param requestUri The requestUri to build
     * @return ESF_SUCCESS if successful, ESF_AGAIN if more data needs to be read,
     *  another error code otherwise.
     */
    ESFError parse(ESFBuffer *inputBuffer, AWSHttpRequestUri *requestUri);

private:
    // Disabled
    AWSHttpRequestUriParser(const AWSHttpRequestUriParser &);
    void operator=(const AWSHttpRequestUriParser &);

    // "*"
    ESFError parseAsterisk(ESFBuffer *inputBuffer, AWSHttpRequestUri *requestUri);

    // abs_path      = "/"  path_segments
    // path_segments = segment *( "/" segment )
    // segment       = *pchar *( ";" param )
    // param         = *pchar
    ESFError parseAbsPath(ESFBuffer *inputBuffer, AWSHttpRequestUri *requestUri);

    // query         = *uric
    ESFError parseQuery(ESFBuffer *inputBuffer, AWSHttpRequestUri *requestUri);

    // absoluteURI   = scheme ":" ( hier_part | opaque_part )
    // scheme        = alpha *( alpha | digit | "+" | "-" | "." )
    ESFError parseScheme(ESFBuffer *inputBuffer, AWSHttpRequestUri *requestUri);

    // host          = hostname | IPv4address
    // hostname      = *( domainlabel "." ) toplabel [ "." ]
    // domainlabel   = alphanum | alphanum *( alphanum | "-" ) alphanum
    // toplabel      = alpha | alpha *( alphanum | "-" ) alphanum
    // IPv4address   = 1*digit "." 1*digit "." 1*digit "." 1*digit
    ESFError parseHost(ESFBuffer *inputBuffer, AWSHttpRequestUri *requestUri);

    // port          = *digit
    ESFError parsePort(ESFBuffer *inputBuffer, AWSHttpRequestUri *requestUri);

    // fragment      = *uric
    ESFError parseFragment(ESFBuffer *inputBuffer, AWSHttpRequestUri *requestUri);

    // absoluteURI   = scheme ":" ( hier_part | opaque_part )
    // hier_part     = ( net_path | abs_path ) [ "?" query ]
    // net_path      = "//" authority [ abs_path ]
    // abs_path      = "/"  path_segments
    // opaque_part   = uric_no_slash *uric
    // uric_no_slash = unreserved | escaped | ";" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | ","
    ESFError parseNonHttpUri(ESFBuffer *inputBuffer, AWSHttpRequestUri *requestUri);

    ESFError skipForwardSlashes(ESFBuffer *inputBuffer, AWSHttpRequestUri *requestUri);

    int _state;
    ESFBuffer *_workingBuffer;
    ESFDiscardAllocator *_allocator;
};

#endif
