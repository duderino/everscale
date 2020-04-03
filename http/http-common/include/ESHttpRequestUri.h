#ifndef ES_HTTP_REQUEST_URI_H
#define ES_HTTP_REQUEST_URI_H

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

namespace ES {

/**
 * A HTTP Request-URI as defined in RFC 2616 and RFC 2396
 *
 * TODO add function to compare uris with deeply parsed abs_path and query
 * strings.
 */
class HttpRequestUri {
 public:
  typedef enum {
    ES_URI_ASTERISK = 0, /**< OPTIONS * HTTP/1.1 */
    ES_URI_HTTP = 1,     /**< GET http://www.yahoo.com/ HTTP/1.1 */
    ES_URI_HTTPS = 2,    /**< POST https://www.yahoo.com/ HTTP/1.1 */
    ES_URI_OTHER = 3     /**< POST foo://opaque */
  } UriType;

  HttpRequestUri(UriType type);

  HttpRequestUri();

  void reset();

  virtual ~HttpRequestUri();

  /**
   * Get the Request-URI's type
   *
   * @return The Request-URI's type
   */
  inline UriType type() const { return _type; }

  /**
   * Set the Request-URI's type
   *
   * @param type the Request-URI's type
   */
  inline void setType(UriType type) { _type = type; }

  /**
   * Get the Request-URI's abs-path.  Example:
   * <p/>The "/bar/baz" in
   * "https://www.foo.com:443/bar/baz?id=70029023&trkid=134852"
   *
   * @return The Request-URI's abs-path or NULL if not set
   */
  inline const unsigned char *absPath() const { return _absPath; }

  /**
   * Set the Request-URI's abs-path.  Caller controls the memory.
   *
   * @param absPath The Request-URI's abs-path
   */
  inline void setAbsPath(const unsigned char *absPath) { _absPath = absPath; }

  /**
   * Set the Request-URI's abs-path.  Caller controls the memory.
   *
   * @param absPath The Request-URI's abs-path
   */
  inline void setAbsPath(const char *absPath) {
    _absPath = (const unsigned char *)absPath;
  }

  /**
   * Get the Request-URI's query string if set.  Example:
   * <p/>The "id=70029023&trkid=134852" in
   * "http://www.foo.com/bar/baz?id=70029023&trkid=134852"
   *
   * @return the Request-URI's query string. or NULL if not set
   */
  inline const unsigned char *query() const { return _query; }

  /**
   * Set the Request-URI's query string.  Caller controls memory.
   *
   * @param query The Request-URI's query string
   */
  inline void setQuery(const unsigned char *query) { _query = query; }

  /**
   * Set the Request-URI's query string.  Caller controls memory.
   *
   * @param query The Request-URI's query string
   */
  inline void setQuery(const char *query) {
    _query = (const unsigned char *)query;
  }

  /**
   * Get the Request-URI's host.
   * Example:
   * <p/>The "www.foo.com" in "https://www.foo.com:443/bar/baz"
   *
   * @return The Request-URI's host or NULL if not set.
   */
  inline const unsigned char *host() const { return _host; }

  /**
   * Set the Request-URI's host.  Caller controls memory.
   *
   * @param host The Request-URI's host
   */
  inline void setHost(const unsigned char *host) { _host = host; }

  /**
   * Set the Request-URI's host.  Caller controls memory.
   *
   * @param host The Request-URI's host
   */
  inline void setHost(const char *host) { _host = (const unsigned char *)host; }

  /**
   * Get the Request-URI's port number if it is set.
   * Example:
   * <p/>The "443" in "https://www.foo.com:443/bar/baz"
   *
   * @return The Request-URI's port number if it is set, -1 otherwise.
   */
  inline ESB::Int32 port() const { return _port; }

  /**
   * Set the Request-URI's port number
   *
   * @param port The port
   */
  inline void setPort(ESB::Int32 port) { _port = port; }

  /**
   * Get the Request-URI's fragment (anchor)
   *
   * @return The Request-URI's fragement if set, NULL otherwise.
   */
  inline const unsigned char *fragment() const { return _fragment; }

  /**
   * Set the Request-URI's fragment (anchor)
   *
   * @param fragment The Request-URI's fragement.  Caller controls memory.
   */
  inline void setFragment(const unsigned char *fragment) {
    _fragment = fragment;
  }

  /**
   * Set the Request-URI's fragment (anchor)
   *
   * @param fragment The Request-URI's fragement.  Caller controls memory.
   */
  inline void setFragment(const char *fragment) {
    _fragment = (const unsigned char *)fragment;
  }

  /**
   * Get the raw form of a non-http, non-https uri.
   *
   * @return The raw Request-URI
   */
  inline const unsigned char *other() const { return _other; }

  /**
   * Set the raw form of a non-http, non-https uri.
   *
   * @param other The raw Request-URI
   */
  inline void setOther(const unsigned char *other) { _other = other; }

  /**
   * Compare two Request-URIs for equality according to RFC 2616.
   * The fragment does not influence the comparison.
   * <p/>
   * 3.2.3 URI Comparison
   * <p/>
   * When comparing two URIs to decide if they match or not, a client
   * SHOULD use a case-sensitive octet-by-octet comparison of the entire
   * URIs, with these exceptions:
   * <p/>
   * - A port that is empty or not given is equivalent to the default
   * port for that URI-reference;
   * <p/>
   * - Comparisons of host names MUST be case-insensitive;
   * <p/>
   * - Comparisons of scheme names MUST be case-insensitive;
   * <p/>
   * - An empty abs_path is equivalent to an abs_path of "/".
   * <p/>
   * Characters other than those in the "reserved" and "unsafe" sets (see
   * RFC 2396 [42]) are equivalent to their ""%" HEX HEX" encoding.
   * <p/>
   * For example, the following three URIs are equivalent:
   * <p/>
   * http://abc.com:80/~smith/home.html
   * http://ABC.com/%7Esmith/home.html
   * http://ABC.com:/%7esmith/home.html
   *
   * @param r1 The first request uri in the comparison
   * @param r2 The second request uri in the comparison
   * @return 0 if r1 == r2, negative if r1 < r2, positive if r1 > r2
   */
  static int Compare(const HttpRequestUri *r1, const HttpRequestUri *r2);

 private:
  // Disabled
  HttpRequestUri(const HttpRequestUri &);
  void operator=(const HttpRequestUri &);

  UriType _type;
  ESB::Int32 _port;
  const unsigned char *_host;
  const unsigned char *_absPath;
  const unsigned char *_query;
  const unsigned char *_fragment;
  const unsigned char *_other;
  char _pad[8];
};

}  // namespace ES

#endif
