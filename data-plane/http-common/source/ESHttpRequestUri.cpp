#ifndef ES_HTTP_REQUEST_URI_H
#include <ESHttpRequestUri.h>
#endif

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ES_HTTP_UTIL_H
#include <ESHttpUtil.h>
#endif

namespace ES {

HttpRequestUri::HttpRequestUri(UriType type)
    : _type(type), _port(-1), _host(NULL), _absPath(NULL), _query(NULL), _fragment(NULL), _other(NULL) {}

HttpRequestUri::HttpRequestUri()
    : _type(ES_URI_HTTP), _port(-1), _host(NULL), _absPath(NULL), _query(NULL), _fragment(NULL), _other(NULL) {}

HttpRequestUri::~HttpRequestUri() {}

void HttpRequestUri::reset() {
  _type = ES_URI_HTTP;
  _port = -1;
  _host = NULL;
  _absPath = NULL;
  _query = NULL;
  _fragment = NULL;
  _other = NULL;
}

ESB::Error HttpRequestUri::copy(const HttpRequestUri *other, ESB::Allocator &allocator) {
  if (!other) {
    return ESB_NULL_POINTER;
  }

  _type = other->type();
  _port = other->port();

  if (other->host()) {
    _host = HttpUtil::Duplicate(&allocator, other->host());
    if (!_host) {
      return ESB_OUT_OF_MEMORY;
    }
  }

  if (other->absPath()) {
    _absPath = HttpUtil::Duplicate(&allocator, other->absPath());
    if (!_absPath) {
      allocator.deallocate((void *)_host);
      _host = NULL;
      return ESB_OUT_OF_MEMORY;
    }
  }

  if (other->query()) {
    _query = HttpUtil::Duplicate(&allocator, other->query());
    if (!_query) {
      allocator.deallocate((void *)_host);
      allocator.deallocate((void *)_absPath);
      _host = NULL;
      _absPath = NULL;
      return ESB_OUT_OF_MEMORY;
    }
  }

  if (other->fragment()) {
    _fragment = HttpUtil::Duplicate(&allocator, other->fragment());
    if (!_fragment) {
      allocator.deallocate((void *)_host);
      allocator.deallocate((void *)_absPath);
      allocator.deallocate((void *)_query);
      _host = NULL;
      _absPath = NULL;
      _query = NULL;
      return ESB_OUT_OF_MEMORY;
    }
  }

  if (other->other()) {
    _other = HttpUtil::Duplicate(&allocator, other->other());
    if (!_other) {
      allocator.deallocate((void *)_host);
      allocator.deallocate((void *)_absPath);
      allocator.deallocate((void *)_query);
      allocator.deallocate((void *)_fragment);
      _host = NULL;
      _absPath = NULL;
      _query = NULL;
      _fragment = NULL;
      return ESB_OUT_OF_MEMORY;
    }
  }

  return ESB_SUCCESS;
}

int HttpRequestUri::Compare(const HttpRequestUri *r1, const HttpRequestUri *r2) {
  if (ES_URI_ASTERISK == r1->type() && ES_URI_ASTERISK == r2->type()) {
    return 0;
  }

  if (ES_URI_OTHER == r1->type() && ES_URI_OTHER == r2->type()) {
    return strcmp((const char *)r1->other(), (const char *)r2->other());
  }

  int result = 0;

  // Comparisons of scheme names MUST be case-insensitive
  if (r1->type() != r2->type()) {
    return ES_URI_HTTP == r1->type() ? 1 : -1;
  }

  // todo add username, password, and anchor to comparison

  // Comparisons of host names MUST be case-insensitive
  if (r1->host() != r2->host()) {
    if (0 == r1->host()) {
      return -1;
    }

    if (0 == r2->host()) {
      return 1;
    }

    result = strcasecmp((const char *)r1->host(), (const char *)r2->host());

    if (0 != result) {
      return result;
    }
  }

  // A port that is empty or not given is equivalent to the default port for
  // that URI-reference;

  int port1 = 0 <= r1->port() ? r1->port() : (ES_URI_HTTP == r1->type() ? 80 : 443);
  int port2 = 0 <= r2->port() ? r2->port() : (ES_URI_HTTP == r2->type() ? 80 : 443);

  if (0 != port1 - port2) {
    return port1 - port2;
  }

  // An empty abs_path is equivalent to an abs_path of "/".

  const char *absPath1 = 0 == r1->absPath() ? "/" : (const char *)r1->absPath();
  const char *absPath2 = 0 == r2->absPath() ? "/" : (const char *)r2->absPath();

  if (absPath1 != absPath2) {
    result = strcmp(absPath1, absPath2);

    if (0 != result) {
      return result;
    }
  }

  if (r1->query() != r2->query()) {
    if (0 == r1->query()) {
      return -1;
    }

    if (0 == r2->query()) {
      return 1;
    }

    result = strcmp((const char *)r1->query(), (const char *)r2->query());

    if (0 != result) {
      return result;
    }
  }

  return 0;
}

}  // namespace ES
