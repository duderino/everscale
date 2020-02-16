/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_REQUEST_URI_FORMATTER_H
#define AWS_HTTP_REQUEST_URI_FORMATTER_H

#ifndef AWS_HTTP_REQUEST_URI_H
#include <AWSHttpRequestUri.h>
#endif

#ifndef ESF_BUFFER_H
#include <ESFBuffer.h>
#endif

/**
 * Formats a HTTP Request-URI as defined in RFC 2616 and RFC 2396
 */
class AWSHttpRequestUriFormatter {
 public:
  /** Create a new uri formatter
   */
  AWSHttpRequestUriFormatter();

  virtual ~AWSHttpRequestUriFormatter();

  /**
   * Reset the formatter
   */
  void reset();

  /**
   * Incrementally format a Request-URI.
   *
   * Request-URI   = "*" | absoluteURI | abs_path [ "?" query ] | authority
   *
   * Caller should:
   * (1) Fill buffer with this method.  If return value is not ESF_AGAIN or
   *     ESF_SUCCESS, break
   * (2) Write buffer to socket
   * (3) If ESF_AGAIN was returned, clear buffer & goto (1), otherwise done.
   *
   * @param outputBuffer The buffer to fill
   * @param requestUri The requestUri to format
   * @return ESF_SUCCESS if successful, ESF_AGAIN if the buffer runs out of
   * space, another error code otherwise.
   */
  ESFError format(ESFBuffer *outputBuffer, const AWSHttpRequestUri *requestUri);

 private:
  // Disabled
  AWSHttpRequestUriFormatter(const AWSHttpRequestUriFormatter &);
  void operator=(const AWSHttpRequestUriFormatter *);

  // "*"
  ESFError formatAsterisk(ESFBuffer *outputBuffer,
                          const AWSHttpRequestUri *requestUri);

  // abs_path      = "/"  path_segments
  // path_segments = segment *( "/" segment )
  // segment       = *pchar *( ";" param )
  // param         = *pchar
  ESFError formatAbsPath(ESFBuffer *outputBuffer,
                         const AWSHttpRequestUri *requestUri);

  // query         = *uric
  ESFError formatQuery(ESFBuffer *outputBuffer,
                       const AWSHttpRequestUri *requestUri);

  // fragment     = *uric
  ESFError formatFragment(ESFBuffer *outputBuffer,
                          const AWSHttpRequestUri *requestUri);

  // http_URL       = "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]
  // absoluteURI   = scheme ":" ( hier_part | opaque_part )
  // scheme        = alpha *( alpha | digit | "+" | "-" | "." )
  ESFError formatScheme(ESFBuffer *outputBuffer,
                        const AWSHttpRequestUri *requestUri);

  // host          = hostname | IPv4address
  // hostname      = *( domainlabel "." ) toplabel [ "." ]
  // domainlabel   = alphanum | alphanum *( alphanum | "-" ) alphanum
  // toplabel      = alpha | alpha *( alphanum | "-" ) alphanum
  // IPv4address   = 1*digit "." 1*digit "." 1*digit "." 1*digit
  ESFError formatHost(ESFBuffer *outputBuffer,
                      const AWSHttpRequestUri *requestUri);

  // port          = *digit
  ESFError formatPort(ESFBuffer *outputBuffer,
                      const AWSHttpRequestUri *requestUri);

  // absoluteURI   = scheme ":" ( hier_part | opaque_part )
  // hier_part     = ( net_path | abs_path ) [ "?" query ]
  // net_path      = "//" authority [ abs_path ]
  // abs_path      = "/"  path_segments
  // opaque_part   = uric_no_slash *uric
  // uric_no_slash = unreserved | escaped | ";" | "?" | ":" | "@" | "&" | "=" |
  // "+" | "$" | ","
  ESFError formatNonHttpUri(ESFBuffer *outputBuffer,
                            const AWSHttpRequestUri *requestUri);

  int _state;
};

#endif
