#ifndef ES_HTTP_REQUEST_URI_PARSER_H
#include <ESHttpRequestUriParser.h>
#endif

#ifndef ES_HTTP_UTIL_H
#include <ESHttpUtil.h>
#endif

#ifndef ES_HTTP_ERROR_H
#include <ESHttpError.h>
#endif

namespace ES {

#define HTTP ((const unsigned char *)"http")
#define HTTPS ((const unsigned char *)"https")

#define ES_URI_PARSING_ASTERISK (1 << 0)
#define ES_URI_PARSING_SCHEME (1 << 1)
#define ES_URI_PARSING_HOST (1 << 2)
#define ES_URI_PARSING_PORT (1 << 3)
#define ES_URI_PARSING_ABS_PATH (1 << 4)
#define ES_URI_PARSING_QUERY (1 << 5)
#define ES_URI_PARSING_FRAGMENT (1 << 6)
#define ES_URI_PARSE_COMPLETE (1 << 7)
#define ES_URI_PARSING_NON_HTTP_URI (1 << 8)
#define ES_URI_SKIPPING_FWD_SLASHES (1 << 9)

HttpRequestUriParser::HttpRequestUriParser(ESB::Buffer *parseBuffer,
                                           ESB::DiscardAllocator &allocator)
    : _state(0x00), _parseBuffer(parseBuffer), _allocator(allocator) {}

HttpRequestUriParser::~HttpRequestUriParser() {}

void HttpRequestUriParser::reset() {
  _state = 0x00;
  _parseBuffer->clear();
}

ESB::Error HttpRequestUriParser::parse(ESB::Buffer *inputBuffer,
                                       HttpRequestUri &requestUri) {
  // Request-URI   = "*" | absoluteURI | abs_path [ "?" query ] | authority

  if (ES_URI_PARSE_COMPLETE & _state) {
    return ESB_INVALID_STATE;
  }

  if (0x00 == _state) {
    // Clients SHOULD be tolerant in parsing the Status-Line and servers
    // tolerant when parsing the Request-Line. In particular, they SHOULD
    // accept any amount of SP or HT characters between fields, even though
    // only a single SP is required.

    HttpUtil::SkipSpaces(inputBuffer);

    inputBuffer->readMark();

    if (!inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    switch (inputBuffer->peekNext()) {
      case '*':
        _state = ES_URI_PARSING_ASTERISK;
        break;

      case '/':
        _state = ES_URI_PARSING_ABS_PATH;
        break;

      default:
        _state = ES_URI_PARSING_SCHEME;
    }
  }

  // http_URL       = "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]

  if (ES_URI_PARSING_ASTERISK & _state) {
    return parseAsterisk(inputBuffer, requestUri);
  }

  if (ES_URI_PARSING_SCHEME & _state) {
    return parseScheme(inputBuffer, requestUri);
  }

  if (ES_URI_SKIPPING_FWD_SLASHES & _state) {
    return skipForwardSlashes(inputBuffer, requestUri);
  }

  if (ES_URI_PARSING_HOST & _state) {
    return parseHost(inputBuffer, requestUri);
  }

  if (ES_URI_PARSING_PORT & _state) {
    return parsePort(inputBuffer, requestUri);
  }

  if (ES_URI_PARSING_ABS_PATH & _state) {
    return parseAbsPath(inputBuffer, requestUri);
  }

  if (ES_URI_PARSING_QUERY & _state) {
    return parseQuery(inputBuffer, requestUri);
  }

  if (ES_URI_PARSING_FRAGMENT & _state) {
    return parseFragment(inputBuffer, requestUri);
  }

  if (ES_URI_PARSING_NON_HTTP_URI & _state) {
    return parseNonHttpUri(inputBuffer, requestUri);
  }

  return ESB_INVALID_STATE;
}

ESB::Error HttpRequestUriParser::parseAsterisk(ESB::Buffer *inputBuffer,
                                               HttpRequestUri &requestUri) {
  assert(ES_URI_PARSING_ASTERISK & _state);

  if (!inputBuffer->isReadable()) {
    return ESB_AGAIN;
  }

  if ('*' != inputBuffer->next()) {
    return ES_HTTP_BAD_REQUEST_URI_ASTERISK;
  }

  if (!inputBuffer->isReadable()) {
    inputBuffer->readReset();

    return ESB_AGAIN;
  }

  if (!HttpUtil::IsSpace(inputBuffer->next())) {
    return ES_HTTP_BAD_REQUEST_URI_ASTERISK;
  }

  requestUri.setType(HttpRequestUri::ES_URI_ASTERISK);

  _state &= ~ES_URI_PARSING_ASTERISK;
  _state |= ES_URI_PARSE_COMPLETE;

  return ESB_SUCCESS;
}

ESB::Error HttpRequestUriParser::parseAbsPath(ESB::Buffer *inputBuffer,
                                              HttpRequestUri &requestUri) {
  // abs_path      = "/"  path_segments
  // path_segments = segment *( "/" segment )
  // segment       = *pchar *( ";" param )
  // param         = *pchar

  assert(ES_URI_PARSING_ABS_PATH & _state);

  unsigned char octet;

  if (0 == _parseBuffer->writePosition()) {
    if (!inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    if (!_parseBuffer->isWritable()) {
      return ESB_OVERFLOW;
    }

    octet = inputBuffer->next();

    if ('/' != octet) {
      return ES_HTTP_BAD_REQUEST_URI_ABS_PATH;
    }

    _parseBuffer->putNext('/');
  }

  // ESB::Error error;

  while (true) {
    if (!inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    if (!_parseBuffer->isWritable()) {
      return ESB_OVERFLOW;
    }

    octet = inputBuffer->next();

    /** DO NOT DECODE - a full parse of the absolute path is required to do this
     *  properly:

    if ('%' == octet)
    {
        inputBuffer->setReadPosition(inputBuffer->readPosition() - 1);

        error = HttpUtil::DecodeEscape(inputBuffer, &octet);

        if (ESB_AGAIN == error)
        {
            return ESB_AGAIN;
        }

        if (ESB_SUCCESS != error)
        {
            return ES_HTTP_BAD_REQUEST_URI_ABS_PATH;
        }

        _workingBuffer->putNext(octet);
        continue;
    } */

    if (HttpUtil::IsPchar(octet)) {
      _parseBuffer->putNext(octet);
      continue;
    }

    if ('/' == octet || ';' == octet) {
      _parseBuffer->putNext(octet);
      continue;
    }

    if ('?' == octet) {
      requestUri.setAbsPath(_parseBuffer->duplicate(_allocator));

      if (0 == requestUri.absPath()) {
        return ESB_OUT_OF_MEMORY;
      }

      inputBuffer->readMark();
      _parseBuffer->clear();

      _state &= ~ES_URI_PARSING_ABS_PATH;
      _state |= ES_URI_PARSING_QUERY;

      return parseQuery(inputBuffer, requestUri);
    }

    if ('#' == octet) {
      requestUri.setAbsPath(_parseBuffer->duplicate(_allocator));

      if (0 == requestUri.absPath()) {
        return ESB_OUT_OF_MEMORY;
      }

      inputBuffer->readMark();
      _parseBuffer->clear();

      _state &= ~ES_URI_PARSING_ABS_PATH;
      _state |= ES_URI_PARSING_FRAGMENT;

      return parseFragment(inputBuffer, requestUri);
    }

    if (HttpUtil::IsSpace(octet)) {
      requestUri.setAbsPath(_parseBuffer->duplicate(_allocator));

      if (0 == requestUri.absPath()) {
        return ESB_OUT_OF_MEMORY;
      }

      inputBuffer->readMark();
      _parseBuffer->clear();

      _state &= ~ES_URI_PARSING_ABS_PATH;
      _state |= ES_URI_PARSE_COMPLETE;

      return ESB_SUCCESS;
    }

    return ES_HTTP_BAD_REQUEST_URI_ABS_PATH;
  }
}

ESB::Error HttpRequestUriParser::parseQuery(ESB::Buffer *inputBuffer,
                                            HttpRequestUri &requestUri) {
  // query         = *uric

  assert(ES_URI_PARSING_QUERY & _state);

  unsigned char octet;
  // ESB::Error error;

  while (true) {
    if (!inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    if (!_parseBuffer->isWritable()) {
      return ESB_OVERFLOW;
    }

    octet = inputBuffer->next();

    /* DO NOT DECODE - A full parse of the query string parameters is
     * required to do this properly:

    if ('%' == octet)
    {
        inputBuffer->setReadPosition(inputBuffer->readPosition() - 1);

        error = HttpUtil::DecodeEscape(inputBuffer, &octet);

        if (ESB_AGAIN == error)
        {
            return ESB_AGAIN;
        }

        if (ESB_SUCCESS != error)
        {
            return ES_HTTP_BAD_REQUEST_URI_QUERY;
        }

        _workingBuffer->putNext(octet);
        continue;
    }*/

    if (HttpUtil::IsUric(octet)) {
      _parseBuffer->putNext(octet);
      continue;
    }

    if ('#' == octet) {
      requestUri.setQuery(_parseBuffer->duplicate(_allocator));

      if (0 == requestUri.query()) {
        return ESB_OUT_OF_MEMORY;
      }

      inputBuffer->readMark();
      _parseBuffer->clear();

      _state &= ~ES_URI_PARSING_QUERY;
      _state |= ES_URI_PARSING_FRAGMENT;

      return parseFragment(inputBuffer, requestUri);
    }

    if (HttpUtil::IsSpace(octet)) {
      requestUri.setQuery(_parseBuffer->duplicate(_allocator));

      if (0 == requestUri.query()) {
        return ESB_OUT_OF_MEMORY;
      }

      inputBuffer->readMark();
      _parseBuffer->clear();

      _state &= ~ES_URI_PARSING_QUERY;
      _state |= ES_URI_PARSE_COMPLETE;

      return ESB_SUCCESS;
    }

    return ES_HTTP_BAD_REQUEST_URI_QUERY;
  }
}

ESB::Error HttpRequestUriParser::parseFragment(ESB::Buffer *inputBuffer,
                                               HttpRequestUri &requestUri) {
  // fragment     = *uric

  assert(ES_URI_PARSING_FRAGMENT & _state);

  unsigned char octet;
  // ESB::Error error;

  while (true) {
    if (!inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    if (!_parseBuffer->isWritable()) {
      return ESB_OVERFLOW;
    }

    octet = inputBuffer->next();

    /* DO NOT DECODE - interpretation of the fragment is up to the application

    if ('%' == octet)
    {
        inputBuffer->setReadPosition(inputBuffer->readPosition() - 1);

        error = HttpUtil::DecodeEscape(inputBuffer, &octet);

        if (ESB_AGAIN == error)
        {
            return ESB_AGAIN;
        }

        if (ESB_SUCCESS != error)
        {
            return ES_HTTP_BAD_REQUEST_URI_QUERY;
        }

        _workingBuffer->putNext(octet);
        continue;
    }*/

    if (HttpUtil::IsUric(octet)) {
      _parseBuffer->putNext(octet);
      continue;
    }

    if (HttpUtil::IsSpace(octet)) {
      requestUri.setFragment(_parseBuffer->duplicate(_allocator));

      if (0 == requestUri.fragment()) {
        return ESB_OUT_OF_MEMORY;
      }

      inputBuffer->readMark();
      _parseBuffer->clear();

      _state &= ~ES_URI_PARSING_FRAGMENT;
      _state |= ES_URI_PARSE_COMPLETE;

      return ESB_SUCCESS;
    }

    return ES_HTTP_BAD_REQUEST_URI_FRAGMENT;
  }
}

ESB::Error HttpRequestUriParser::parseScheme(ESB::Buffer *inputBuffer,
                                             HttpRequestUri &requestUri) {
  // http_URL       = "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]
  // absoluteURI   = scheme ":" ( hier_part | opaque_part )
  // scheme        = alpha *( alpha | digit | "+" | "-" | "." )

  assert(ES_URI_PARSING_SCHEME & _state);

  unsigned char octet;

  if (0 == _parseBuffer->writePosition()) {
    if (!inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    if (!_parseBuffer->isWritable()) {
      return ESB_OVERFLOW;
    }

    octet = inputBuffer->next();

    if (!HttpUtil::IsAlpha(octet)) {
      return ES_HTTP_BAD_REQUEST_URI_SCHEME;
    }

    _parseBuffer->putNext(octet);
  }

  while (true) {
    if (!inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    if (!_parseBuffer->isWritable()) {
      return ESB_OVERFLOW;
    }

    octet = inputBuffer->next();

    if (HttpUtil::IsAlphaNum(octet)) {
      _parseBuffer->putNext(octet);
      continue;
    }

    switch (octet) {
      case '+':
      case '=':
      case '.':

        _parseBuffer->putNext(octet);
        break;

      case ':':

        inputBuffer->readMark();

        _state &= ~ES_URI_PARSING_SCHEME;

        if (_parseBuffer->match(HTTP)) {
          _state |= ES_URI_SKIPPING_FWD_SLASHES;

          requestUri.setType(HttpRequestUri::ES_URI_HTTP);

          _parseBuffer->clear();

          return skipForwardSlashes(inputBuffer, requestUri);
        } else if (_parseBuffer->match(HTTPS)) {
          _state |= ES_URI_SKIPPING_FWD_SLASHES;

          requestUri.setType(HttpRequestUri::ES_URI_HTTPS);

          _parseBuffer->clear();

          return skipForwardSlashes(inputBuffer, requestUri);
        } else {
          _state |= ES_URI_PARSING_NON_HTTP_URI;

          requestUri.setType(HttpRequestUri::ES_URI_OTHER);

          _parseBuffer->putNext(':');

          return parseNonHttpUri(inputBuffer, requestUri);
        }

      default:

        return ES_HTTP_BAD_REQUEST_URI_SCHEME;
    }
  }
}

ESB::Error HttpRequestUriParser::skipForwardSlashes(
    ESB::Buffer *inputBuffer, HttpRequestUri &requestUri) {
  // Skips the "//" in ...
  // http_URL       = "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]

  assert(ES_URI_SKIPPING_FWD_SLASHES & _state);

  while (true) {
    if (!inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    if ('/' == inputBuffer->peekNext()) {
      inputBuffer->skipNext();
      continue;
    }

    _state &= ~ES_URI_SKIPPING_FWD_SLASHES;
    _state |= ES_URI_PARSING_HOST;

    return parseHost(inputBuffer, requestUri);
  }
}

ESB::Error HttpRequestUriParser::parseHost(ESB::Buffer *inputBuffer,
                                           HttpRequestUri &requestUri) {
  // host          = hostname | IPv4address
  // hostname      = *( domainlabel "." ) toplabel [ "." ]
  // domainlabel   = alphanum | alphanum *( alphanum | "-" ) alphanum
  // toplabel      = alpha | alpha *( alphanum | "-" ) alphanum
  // IPv4address   = 1*digit "." 1*digit "." 1*digit "." 1*digit

  // todo whoops username and password may also be present here!  which spec
  // controls this? todo the parseHost code is too permissive

  assert(ES_URI_PARSING_HOST & _state);

  unsigned char octet;

  if (0 == _parseBuffer->writePosition()) {
    if (!inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    if (!_parseBuffer->isWritable()) {
      return ESB_OVERFLOW;
    }

    octet = inputBuffer->next();

    if (!HttpUtil::IsAlphaNum(octet)) {
      return ES_HTTP_BAD_REQUEST_URI_HOST;
    }

    _parseBuffer->putNext(octet);
  }

  while (true) {
    if (!inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    if (!_parseBuffer->isWritable()) {
      return ESB_OVERFLOW;
    }

    octet = inputBuffer->next();

    if (HttpUtil::IsAlphaNum(octet)) {
      _parseBuffer->putNext(octet);
      continue;
    }

    switch (octet) {
      case '-':
      case '.':

        _parseBuffer->putNext(octet);
        break;

      case ':':

        requestUri.setHost(_parseBuffer->duplicate(_allocator));

        if (0 == requestUri.host()) {
          return ESB_OUT_OF_MEMORY;
        }

        inputBuffer->readMark();
        _parseBuffer->clear();

        _state &= ~ES_URI_PARSING_HOST;
        _state |= ES_URI_PARSING_PORT;

        return parsePort(inputBuffer, requestUri);

      case ' ':
      case '\t':

        requestUri.setHost(_parseBuffer->duplicate(_allocator));

        if (0 == requestUri.host()) {
          return ESB_OUT_OF_MEMORY;
        }

        inputBuffer->readMark();
        _parseBuffer->clear();

        _state &= ~ES_URI_PARSING_HOST;
        _state |= ES_URI_PARSE_COMPLETE;

        requestUri.setAbsPath("/");

        return ESB_SUCCESS;

      case '/':

        requestUri.setHost(_parseBuffer->duplicate(_allocator));

        if (0 == requestUri.host()) {
          return ESB_OUT_OF_MEMORY;
        }

        inputBuffer->setReadPosition(inputBuffer->readPosition() - 1);
        inputBuffer->readMark();
        _parseBuffer->clear();

        _state &= ~ES_URI_PARSING_HOST;
        _state |= ES_URI_PARSING_ABS_PATH;

        return parseAbsPath(inputBuffer, requestUri);

      case '?':

        requestUri.setHost(_parseBuffer->duplicate(_allocator));

        if (0 == requestUri.host()) {
          return ESB_OUT_OF_MEMORY;
        }

        inputBuffer->readMark();
        _parseBuffer->clear();

        _state &= ~ES_URI_PARSING_HOST;
        _state |= ES_URI_PARSING_QUERY;

        requestUri.setAbsPath("/");

        return parseQuery(inputBuffer, requestUri);

      case '#':

        requestUri.setHost(_parseBuffer->duplicate(_allocator));

        if (0 == requestUri.host()) {
          return ESB_OUT_OF_MEMORY;
        }

        inputBuffer->readMark();
        _parseBuffer->clear();

        _state &= ~ES_URI_PARSING_HOST;
        _state |= ES_URI_PARSING_FRAGMENT;

        requestUri.setAbsPath("/");

        return parseFragment(inputBuffer, requestUri);

      default:

        return ES_HTTP_BAD_REQUEST_URI_HOST;
    }
  }
}

ESB::Error HttpRequestUriParser::parsePort(ESB::Buffer *inputBuffer,
                                           HttpRequestUri &requestUri) {
  // port          = *digit

  assert(ES_URI_PARSING_PORT & _state);

  unsigned char octet;

  while (true) {
    if (!inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    octet = inputBuffer->next();

    if (HttpUtil::IsDigit(octet)) {
      if (0 > requestUri.port()) {
        requestUri.setPort(0);
      }

      requestUri.setPort(requestUri.port() * 10 + (octet - '0'));

      if (65536 <= requestUri.port()) {
        return ES_HTTP_BAD_REQUEST_URI_PORT;
      }

      continue;
    }

    if (HttpUtil::IsSpace(octet)) {
      if (0 > requestUri.port()) {
        return ES_HTTP_BAD_REQUEST_URI_PORT;
      }

      inputBuffer->readMark();
      _parseBuffer->clear();

      _state &= ~ES_URI_PARSING_PORT;
      _state |= ES_URI_PARSE_COMPLETE;

      requestUri.setAbsPath("/");

      return ESB_SUCCESS;
    }

    if ('/' == octet) {
      if (0 > requestUri.port()) {
        return ES_HTTP_BAD_REQUEST_URI_PORT;
      }

      _state &= ~ES_URI_PARSING_PORT;
      _state |= ES_URI_PARSING_ABS_PATH;

      inputBuffer->setReadPosition(inputBuffer->readPosition() - 1);
      inputBuffer->readMark();
      _parseBuffer->clear();

      return parseAbsPath(inputBuffer, requestUri);
    }

    if ('?' == octet) {
      if (0 > requestUri.port()) {
        return ES_HTTP_BAD_REQUEST_URI_PORT;
      }

      _state &= ~ES_URI_PARSING_PORT;
      _state |= ES_URI_PARSING_QUERY;

      inputBuffer->readMark();
      _parseBuffer->clear();

      requestUri.setAbsPath("/");

      return parseQuery(inputBuffer, requestUri);
    }

    if ('#' == octet) {
      if (0 > requestUri.port()) {
        return ES_HTTP_BAD_REQUEST_URI_PORT;
      }

      _state &= ~ES_URI_PARSING_PORT;
      _state |= ES_URI_PARSING_FRAGMENT;

      inputBuffer->readMark();
      _parseBuffer->clear();

      requestUri.setAbsPath("/");

      return parseFragment(inputBuffer, requestUri);
    }

    return ES_HTTP_BAD_REQUEST_URI_PORT;
  }
}

ESB::Error HttpRequestUriParser::parseNonHttpUri(ESB::Buffer *inputBuffer,
                                                 HttpRequestUri &requestUri) {
  // absoluteURI   = scheme ":" ( hier_part | opaque_part )
  // hier_part     = ( net_path | abs_path ) [ "?" query ]
  // net_path      = "//" authority [ abs_path ]
  // abs_path      = "/"  path_segments
  // opaque_part   = uric_no_slash *uric
  // uric_no_slash = unreserved | escaped | ";" | "?" | ":" | "@" | "&" | "=" |
  // "+" | "$" | ","

  assert(ES_URI_PARSING_NON_HTTP_URI & _state);

  unsigned char octet;

  while (true) {
    if (!inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    if (!_parseBuffer->isWritable()) {
      return ESB_OVERFLOW;
    }

    octet = inputBuffer->next();

    if (HttpUtil::IsSpace(octet)) {
      requestUri.setOther(_parseBuffer->duplicate(_allocator));

      if (0 == requestUri.other()) {
        return ESB_OUT_OF_MEMORY;
      }

      inputBuffer->readMark();
      _parseBuffer->clear();

      _state &= ~ES_URI_PARSING_NON_HTTP_URI;
      _state |= ES_URI_PARSE_COMPLETE;

      return ESB_SUCCESS;
    }

    _parseBuffer->putNext(octet);
  }
}

}  // namespace ES
