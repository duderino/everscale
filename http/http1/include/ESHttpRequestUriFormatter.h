#ifndef ES_HTTP_REQUEST_URI_FORMATTER_H
#define ES_HTTP_REQUEST_URI_FORMATTER_H

#ifndef ES_HTTP_REQUEST_URI_H
#include <ESHttpRequestUri.h>
#endif

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

namespace ES {

/**
 * Formats a HTTP Request-URI as defined in RFC 2616 and RFC 2396
 */
class HttpRequestUriFormatter {
 public:
  /** Create a new uri formatter
   */
  HttpRequestUriFormatter();

  virtual ~HttpRequestUriFormatter();

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
   * (1) Fill buffer with this method.  If return value is not ESB_AGAIN or
   *     ESB_SUCCESS, break
   * (2) Write buffer to socket
   * (3) If ESB_AGAIN was returned, clear buffer & goto (1), otherwise done.
   *
   * @param outputBuffer The buffer to fill
   * @param requestUri The requestUri to format
   * @return ESB_SUCCESS if successful, ESB_AGAIN if the buffer runs out of
   * space, another error code otherwise.
   */
  ESB::Error format(ESB::Buffer *outputBuffer,
                    const HttpRequestUri *requestUri);

 private:
  // Disabled
  HttpRequestUriFormatter(const HttpRequestUriFormatter &);
  void operator=(const HttpRequestUriFormatter *);

  // "*"
  ESB::Error formatAsterisk(ESB::Buffer *outputBuffer,
                            const HttpRequestUri *requestUri);

  // abs_path      = "/"  path_segments
  // path_segments = segment *( "/" segment )
  // segment       = *pchar *( ";" param )
  // param         = *pchar
  ESB::Error formatAbsPath(ESB::Buffer *outputBuffer,
                           const HttpRequestUri *requestUri);

  // query         = *uric
  ESB::Error formatQuery(ESB::Buffer *outputBuffer,
                         const HttpRequestUri *requestUri);

  // fragment     = *uric
  ESB::Error formatFragment(ESB::Buffer *outputBuffer,
                            const HttpRequestUri *requestUri);

  // http_URL       = "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]
  // absoluteURI   = scheme ":" ( hier_part | opaque_part )
  // scheme        = alpha *( alpha | digit | "+" | "-" | "." )
  ESB::Error formatScheme(ESB::Buffer *outputBuffer,
                          const HttpRequestUri *requestUri);

  // host          = hostname | IPv4address
  // hostname      = *( domainlabel "." ) toplabel [ "." ]
  // domainlabel   = alphanum | alphanum *( alphanum | "-" ) alphanum
  // toplabel      = alpha | alpha *( alphanum | "-" ) alphanum
  // IPv4address   = 1*digit "." 1*digit "." 1*digit "." 1*digit
  ESB::Error formatHost(ESB::Buffer *outputBuffer,
                        const HttpRequestUri *requestUri);

  // port          = *digit
  ESB::Error formatPort(ESB::Buffer *outputBuffer,
                        const HttpRequestUri *requestUri);

  // absoluteURI   = scheme ":" ( hier_part | opaque_part )
  // hier_part     = ( net_path | abs_path ) [ "?" query ]
  // net_path      = "//" authority [ abs_path ]
  // abs_path      = "/"  path_segments
  // opaque_part   = uric_no_slash *uric
  // uric_no_slash = unreserved | escaped | ";" | "?" | ":" | "@" | "&" | "=" |
  // "+" | "$" | ","
  ESB::Error formatNonHttpUri(ESB::Buffer *outputBuffer,
                              const HttpRequestUri *requestUri);

  int _state;
};

}  // namespace ES

#endif
