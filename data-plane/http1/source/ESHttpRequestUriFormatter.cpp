#ifndef ES_HTTP_REQUEST_URI_FORMATTER_H
#include <ESHttpRequestUriFormatter.h>
#endif

#ifndef ES_HTTP_ERROR_H
#include <ESHttpError.h>
#endif

#ifndef ES_HTTP_UTIL_H
#include <ESHttpUtil.h>
#endif

namespace ES {

#define ES_URI_FORMATTING_ASTERISK (1 << 0)
#define ES_URI_FORMATTING_SCHEME (1 << 1)
#define ES_URI_FORMATTING_HOST (1 << 2)
#define ES_URI_FORMATTING_PORT (1 << 3)
#define ES_URI_FORMATTING_ABS_PATH (1 << 4)
#define ES_URI_FORMATTING_QUERY (1 << 5)
#define ES_URI_FORMATTING_FRAGMENT (1 << 6)
#define ES_URI_FORMAT_COMPLETE (1 << 7)
#define ES_URI_FORMATTING_NON_HTTP_URI (1 << 8)

HttpRequestUriFormatter::HttpRequestUriFormatter() : _state(0x00) {}

HttpRequestUriFormatter::~HttpRequestUriFormatter() {}

void HttpRequestUriFormatter::reset() { _state = 0x00; }

ESB::Error HttpRequestUriFormatter::format(ESB::Buffer *outputBuffer, const HttpRequestUri &requestUri) {
  // Request-URI   = "*" | absoluteURI | abs_path [ "?" query ] | authority

  if (ES_URI_FORMAT_COMPLETE & _state) {
    return ESB_INVALID_STATE;
  }

  if (0x00 == _state) {
    if (!outputBuffer->isWritable()) {
      return ESB_AGAIN;
    }

    switch (requestUri.type()) {
      case HttpRequestUri::ES_URI_ASTERISK:

        HttpUtil::Start(&_state, outputBuffer, ES_URI_FORMATTING_ASTERISK);

        break;

      case HttpRequestUri::ES_URI_HTTP:
      case HttpRequestUri::ES_URI_HTTPS:

        if (requestUri.host() && 0 != requestUri.host()[0]) {
          HttpUtil::Start(&_state, outputBuffer, ES_URI_FORMATTING_SCHEME);
        } else {
          HttpUtil::Start(&_state, outputBuffer, ES_URI_FORMATTING_ABS_PATH);
        }

        break;

      case HttpRequestUri::ES_URI_OTHER:

        HttpUtil::Start(&_state, outputBuffer, ES_URI_FORMATTING_NON_HTTP_URI);

        break;

      default:

        return HttpUtil::Rollback(outputBuffer, ESB_INVALID_ARGUMENT);
    }
  }

  ESB::Error error;

  if (ES_URI_FORMATTING_ASTERISK & _state) {
    return formatAsterisk(outputBuffer, requestUri);
  }

  if (ES_URI_FORMATTING_NON_HTTP_URI & _state) {
    return formatNonHttpUri(outputBuffer, requestUri);
  }

  if (ES_URI_FORMATTING_SCHEME & _state) {
    error = formatScheme(outputBuffer, requestUri);

    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  if (ES_URI_FORMATTING_HOST & _state) {
    error = formatHost(outputBuffer, requestUri);

    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  if (ES_URI_FORMATTING_PORT & _state) {
    error = formatPort(outputBuffer, requestUri);

    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  if (ES_URI_FORMATTING_ABS_PATH & _state) {
    error = formatAbsPath(outputBuffer, requestUri);

    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  if (ES_URI_FORMATTING_QUERY & _state) {
    error = formatQuery(outputBuffer, requestUri);

    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  if (ES_URI_FORMATTING_FRAGMENT & _state) {
    return formatFragment(outputBuffer, requestUri);
  }

  return HttpUtil::Rollback(outputBuffer, ESB_INVALID_STATE);
}

ESB::Error HttpRequestUriFormatter::formatAsterisk(ESB::Buffer *outputBuffer, const HttpRequestUri &requestUri) {
  // "*"

  assert(ES_URI_FORMATTING_ASTERISK & _state);

  if (outputBuffer->writable() == 0) {
    return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
  }

  outputBuffer->putNext('*');

  return HttpUtil::Transition(&_state, outputBuffer, ES_URI_FORMATTING_ASTERISK, ES_URI_FORMAT_COMPLETE);
}

ESB::Error HttpRequestUriFormatter::formatAbsPath(ESB::Buffer *outputBuffer, const HttpRequestUri &requestUri) {
  // abs_path      = "/"  path_segments
  // path_segments = segment *( "/" segment )
  // segment       = *pchar *( ";" param )
  // param         = *pchar

  assert(ES_URI_FORMATTING_ABS_PATH & _state);

  const unsigned char *p = requestUri.absPath() ? requestUri.absPath() : (const unsigned char *)"/";

  if ('/' != *p) {
    if (!outputBuffer->isWritable()) {
      return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
    }

    outputBuffer->putNext('/');
  }

  ESB::Error error;

  for (; *p; ++p) {
    if (!outputBuffer->isWritable()) {
      return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
    }

    if (HttpUtil::IsPchar(*p)) {
      outputBuffer->putNext(*p);
      continue;
    }

    if ('/' == *p || ';' == *p) {
      outputBuffer->putNext(*p);
      continue;
    }

    error = HttpUtil::EncodeEscape(outputBuffer, *p);

    if (ESB_SUCCESS != error) {
      return HttpUtil::Rollback(outputBuffer, error);
    }
  }

  return HttpUtil::Transition(&_state, outputBuffer, ES_URI_FORMATTING_ABS_PATH, ES_URI_FORMATTING_QUERY);
}

ESB::Error HttpRequestUriFormatter::formatQuery(ESB::Buffer *outputBuffer, const HttpRequestUri &requestUri) {
  // query         = *uric

  assert(ES_URI_FORMATTING_QUERY & _state);

  const unsigned char *p = requestUri.query();

  if (!p || 0 == *p) {
    return HttpUtil::Transition(&_state, outputBuffer, ES_URI_FORMATTING_QUERY, ES_URI_FORMATTING_FRAGMENT);
  }

  if (!outputBuffer->isWritable()) {
    return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
  }

  outputBuffer->putNext('?');

  ESB::Error error;

  for (; *p; ++p) {
    if (!outputBuffer->isWritable()) {
      return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
    }

    if (HttpUtil::IsUric(*p)) {
      outputBuffer->putNext(*p);
      continue;
    }

    error = HttpUtil::EncodeEscape(outputBuffer, *p);

    if (ESB_SUCCESS != error) {
      return HttpUtil::Rollback(outputBuffer, error);
    }
  }

  return HttpUtil::Transition(&_state, outputBuffer, ES_URI_FORMATTING_QUERY, ES_URI_FORMATTING_FRAGMENT);
}

ESB::Error HttpRequestUriFormatter::formatFragment(ESB::Buffer *outputBuffer, const HttpRequestUri &requestUri) {
  // fragment         = *uric

  assert(ES_URI_FORMATTING_FRAGMENT & _state);

  const unsigned char *p = requestUri.fragment();

  if (!p || 0 == *p) {
    return HttpUtil::Transition(&_state, outputBuffer, ES_URI_FORMATTING_FRAGMENT, ES_URI_FORMAT_COMPLETE);
  }

  if (!outputBuffer->isWritable()) {
    return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
  }

  outputBuffer->putNext('#');

  ESB::Error error;

  for (; *p; ++p) {
    if (!outputBuffer->isWritable()) {
      return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
    }

    if (HttpUtil::IsUric(*p)) {
      outputBuffer->putNext(*p);
      continue;
    }

    error = HttpUtil::EncodeEscape(outputBuffer, *p);

    if (ESB_SUCCESS != error) {
      return HttpUtil::Rollback(outputBuffer, error);
    }
  }

  return HttpUtil::Transition(&_state, outputBuffer, ES_URI_FORMATTING_FRAGMENT, ES_URI_FORMAT_COMPLETE);
}

ESB::Error HttpRequestUriFormatter::formatScheme(ESB::Buffer *outputBuffer, const HttpRequestUri &requestUri) {
  // http_URL       = "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]
  // absoluteURI   = scheme ":" ( hier_part | opaque_part )
  // scheme        = alpha *( alpha | digit | "+" | "-" | "." )

  assert(ES_URI_FORMATTING_SCHEME & _state);
  assert(HttpRequestUri::ES_URI_ASTERISK != requestUri.type());

  const char *p = 0;

  switch (requestUri.type()) {
    case HttpRequestUri::ES_URI_ASTERISK:

      return HttpUtil::Rollback(outputBuffer, ESB_INVALID_STATE);

    case HttpRequestUri::ES_URI_HTTP:

      p = "http://";

      break;

    case HttpRequestUri::ES_URI_HTTPS:

      p = "https://";

      break;

    case HttpRequestUri::ES_URI_OTHER:

      return HttpUtil::Transition(&_state, outputBuffer, ES_URI_FORMATTING_SCHEME, ES_URI_FORMATTING_HOST);

    default:

      return HttpUtil::Rollback(outputBuffer, ES_HTTP_BAD_REQUEST_URI_SCHEME);
  }

  for (; *p; ++p) {
    if (!outputBuffer->isWritable()) {
      return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
    }

    outputBuffer->putNext((unsigned char)*p);
  }

  return HttpUtil::Transition(&_state, outputBuffer, ES_URI_FORMATTING_SCHEME, ES_URI_FORMATTING_HOST);
}

ESB::Error HttpRequestUriFormatter::formatHost(ESB::Buffer *outputBuffer, const HttpRequestUri &requestUri) {
  // host          = hostname | IPv4address
  // hostname      = *( domainlabel "." ) toplabel [ "." ]
  // domainlabel   = alphanum | alphanum *( alphanum | "-" ) alphanum
  // toplabel      = alpha | alpha *( alphanum | "-" ) alphanum
  // IPv4address   = 1*digit "." 1*digit "." 1*digit "." 1*digit

  assert(ES_URI_FORMATTING_HOST & _state);
  assert(requestUri.host());
  assert(0 != requestUri.host()[0]);
  assert(HttpRequestUri::ES_URI_HTTP == requestUri.type() || HttpRequestUri::ES_URI_HTTPS == requestUri.type());

  for (const unsigned char *p = requestUri.host(); *p; ++p) {
    if (!outputBuffer->isWritable()) {
      return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
    }

    outputBuffer->putNext(*p);  // TODO add validation
  }

  return HttpUtil::Transition(&_state, outputBuffer, ES_URI_FORMATTING_HOST, ES_URI_FORMATTING_PORT);
}

ESB::Error HttpRequestUriFormatter::formatPort(ESB::Buffer *outputBuffer, const HttpRequestUri &requestUri) {
  // port          = *digit

  assert(ES_URI_FORMATTING_PORT & _state);
  assert(requestUri.host());
  assert(0 != requestUri.host()[0]);
  assert(65536 > requestUri.port());
  assert(HttpRequestUri::ES_URI_HTTP == requestUri.type() || HttpRequestUri::ES_URI_HTTPS == requestUri.type());

  ESB::Int32 port = requestUri.port();

  switch (requestUri.type()) {
    case HttpRequestUri::ES_URI_HTTP:

      if (0 > port || 80 == port) {
        return HttpUtil::Transition(&_state, outputBuffer, ES_URI_FORMATTING_PORT, ES_URI_FORMATTING_ABS_PATH);
      }

      break;

    case HttpRequestUri::ES_URI_HTTPS:

      if (0 > port || 443 == port) {
        return HttpUtil::Transition(&_state, outputBuffer, ES_URI_FORMATTING_PORT, ES_URI_FORMATTING_ABS_PATH);
      }

      break;

    default:

      return HttpUtil::Rollback(outputBuffer, ES_HTTP_BAD_REQUEST_URI_PORT);
  }

  assert(0 <= requestUri.port());

  if (65536 <= port) {
    return HttpUtil::Rollback(outputBuffer, ES_HTTP_BAD_REQUEST_URI_PORT);
  }

  if (!outputBuffer->isWritable()) {
    return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
  }

  outputBuffer->putNext(':');

  ESB::Error error = HttpUtil::FormatInteger(outputBuffer, port, 10);

  if (ESB_SUCCESS != error) {
    return HttpUtil::Rollback(outputBuffer, error);
  }

  return HttpUtil::Transition(&_state, outputBuffer, ES_URI_FORMATTING_PORT, ES_URI_FORMATTING_ABS_PATH);
}

ESB::Error HttpRequestUriFormatter::formatNonHttpUri(ESB::Buffer *outputBuffer, const HttpRequestUri &requestUri) {
  // absoluteURI   = scheme ":" ( hier_part | opaque_part )
  // hier_part     = ( net_path | abs_path ) [ "?" query ]
  // net_path      = "//" authority [ abs_path ]
  // abs_path      = "/"  path_segments
  // opaque_part   = uric_no_slash *uric
  // uric_no_slash = unreserved | escaped | ";" | "?" | ":" | "@" | "&" | "=" |
  // "+" | "$" | ","

  assert(ES_URI_FORMATTING_NON_HTTP_URI & _state);
  assert(HttpRequestUri::ES_URI_OTHER == requestUri.type());

  if (0 == requestUri.other() || 0 == requestUri.other()[0]) {
    return HttpUtil::Rollback(outputBuffer, ESB_INVALID_ARGUMENT);
  }

  for (const unsigned char *p = requestUri.other(); *p; ++p) {
    if (!outputBuffer->isWritable()) {
      return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
    }

    outputBuffer->putNext(*p);
  }

  return HttpUtil::Transition(&_state, outputBuffer, ES_URI_FORMATTING_NON_HTTP_URI, ES_URI_FORMAT_COMPLETE);
}

}  // namespace ES
