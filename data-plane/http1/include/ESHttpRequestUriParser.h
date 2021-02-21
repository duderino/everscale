#ifndef ES_HTTP_REQUEST_URI_PARSER_H
#define ES_HTTP_REQUEST_URI_PARSER_H

#ifndef ES_HTTP_REQUEST_URI_H
#include <ESHttpRequestUri.h>
#endif

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

namespace ES {

/**
 * Parses a HTTP Request-URI as defined in RFC 2616 and RFC 2396
 *
 * TODO handle username:password@ in http(s) urls
 * TODO add class/functions to deep parse abs_path (and decode/encode path
 * segements)
 * TODO add class/functions to deep parse query string (and decode/encode query
 * args)
 */
class HttpRequestUriParser {
 public:
  /** Create a new uri parser
   *
   * @param workingBuffer Temporary storage for parsing
   * @param allocator The discard allocator to use for allocating internal
   * strings.
   */
  HttpRequestUriParser(ESB::Buffer *workingBuffer, ESB::DiscardAllocator &allocator);

  virtual ~HttpRequestUriParser();

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
   * (2) Parse buffer with this method. If ESB_AGAIN is returned, compact
   *     buffer.  If error or ESB_SUCCESS returned, break.
   * (3) If no space in buffer, parser is jammed, break and close socket.
   * (4) Otherwise goto (1)
   *
   * @param inputBuffer The buffer to parse
   * @param requestUri The requestUri to build
   * @return ESB_SUCCESS if successful, ESB_AGAIN if more data needs to be read,
   *  another error code otherwise.
   */
  ESB::Error parse(ESB::Buffer *inputBuffer, HttpRequestUri &requestUri);

 private:
  // Disabled
  HttpRequestUriParser(const HttpRequestUriParser &);
  void operator=(const HttpRequestUriParser &);

  // "*"
  ESB::Error parseAsterisk(ESB::Buffer *inputBuffer, HttpRequestUri &requestUri);

  // abs_path      = "/"  path_segments
  // path_segments = segment *( "/" segment )
  // segment       = *pchar *( ";" param )
  // param         = *pchar
  ESB::Error parseAbsPath(ESB::Buffer *inputBuffer, HttpRequestUri &requestUri);

  // query         = *uric
  ESB::Error parseQuery(ESB::Buffer *inputBuffer, HttpRequestUri &requestUri);

  // absoluteURI   = scheme ":" ( hier_part | opaque_part )
  // scheme        = alpha *( alpha | digit | "+" | "-" | "." )
  ESB::Error parseScheme(ESB::Buffer *inputBuffer, HttpRequestUri &requestUri);

  // host          = hostname | IPv4address
  // hostname      = *( domainlabel "." ) toplabel [ "." ]
  // domainlabel   = alphanum | alphanum *( alphanum | "-" ) alphanum
  // toplabel      = alpha | alpha *( alphanum | "-" ) alphanum
  // IPv4address   = 1*digit "." 1*digit "." 1*digit "." 1*digit
  ESB::Error parseHost(ESB::Buffer *inputBuffer, HttpRequestUri &requestUri);

  // port          = *digit
  ESB::Error parsePort(ESB::Buffer *inputBuffer, HttpRequestUri &requestUri);

  // fragment      = *uric
  ESB::Error parseFragment(ESB::Buffer *inputBuffer, HttpRequestUri &requestUri);

  // absoluteURI   = scheme ":" ( hier_part | opaque_part )
  // hier_part     = ( net_path | abs_path ) [ "?" query ]
  // net_path      = "//" authority [ abs_path ]
  // abs_path      = "/"  path_segments
  // opaque_part   = uric_no_slash *uric
  // uric_no_slash = unreserved | escaped | ";" | "?" | ":" | "@" | "&" | "=" |
  // "+" | "$" | ","
  ESB::Error parseNonHttpUri(ESB::Buffer *inputBuffer, HttpRequestUri &requestUri);

  ESB::Error skipForwardSlashes(ESB::Buffer *inputBuffer, HttpRequestUri &requestUri);

  int _state;
  ESB::Buffer *_workingBuffer;
  ESB::DiscardAllocator &_allocator;
};

}  // namespace ES

#endif
