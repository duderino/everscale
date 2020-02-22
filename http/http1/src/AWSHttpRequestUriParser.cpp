/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_REQUEST_URI_PARSER_H
#include <AWSHttpRequestUriParser.h>
#endif

#ifndef AWS_HTTP_UTIL_H
#include <AWSHttpUtil.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifndef AWS_HTTP_ERROR_H
#include <AWSHttpError.h>
#endif

#define HTTP ((const unsigned char *)"http")
#define HTTPS ((const unsigned char *)"https")

#define AWS_URI_PARSING_ASTERISK (1 << 0)
#define AWS_URI_PARSING_SCHEME (1 << 1)
#define AWS_URI_PARSING_HOST (1 << 2)
#define AWS_URI_PARSING_PORT (1 << 3)
#define AWS_URI_PARSING_ABS_PATH (1 << 4)
#define AWS_URI_PARSING_QUERY (1 << 5)
#define AWS_URI_PARSING_FRAGMENT (1 << 6)
#define AWS_URI_PARSE_COMPLETE (1 << 7)
#define AWS_URI_PARSING_NON_HTTP_URI (1 << 8)
#define AWS_URI_SKIPPING_FWD_SLASHES (1 << 9)

AWSHttpRequestUriParser::AWSHttpRequestUriParser(ESFBuffer *workingBuffer,
                                                 ESFDiscardAllocator *allocator)
    : _state(0x00), _workingBuffer(workingBuffer), _allocator(allocator) {}

AWSHttpRequestUriParser::~AWSHttpRequestUriParser() {}

void AWSHttpRequestUriParser::reset() {
  _state = 0x00;
  _workingBuffer->clear();
}

ESFError AWSHttpRequestUriParser::parse(ESFBuffer *inputBuffer,
                                        AWSHttpRequestUri *requestUri) {
  // Request-URI   = "*" | absoluteURI | abs_path [ "?" query ] | authority

  if (AWS_URI_PARSE_COMPLETE & _state) {
    return ESF_INVALID_STATE;
  }

  if (0x00 == _state) {
    // Clients SHOULD be tolerant in parsing the Status-Line and servers
    // tolerant when parsing the Request-Line. In particular, they SHOULD
    // accept any amount of SP or HT characters between fields, even though
    // only a single SP is required.

    AWSHttpUtil::SkipSpaces(inputBuffer);

    inputBuffer->readMark();

    if (false == inputBuffer->isReadable()) {
      return ESF_AGAIN;
    }

    switch (inputBuffer->peekNext()) {
      case '*':
        _state = AWS_URI_PARSING_ASTERISK;
        break;

      case '/':
        _state = AWS_URI_PARSING_ABS_PATH;
        break;

      default:
        _state = AWS_URI_PARSING_SCHEME;
    }
  }

  // http_URL       = "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]

  if (AWS_URI_PARSING_ASTERISK & _state) {
    return parseAsterisk(inputBuffer, requestUri);
  }

  if (AWS_URI_PARSING_SCHEME & _state) {
    return parseScheme(inputBuffer, requestUri);
  }

  if (AWS_URI_SKIPPING_FWD_SLASHES & _state) {
    return skipForwardSlashes(inputBuffer, requestUri);
  }

  if (AWS_URI_PARSING_HOST & _state) {
    return parseHost(inputBuffer, requestUri);
  }

  if (AWS_URI_PARSING_PORT & _state) {
    return parsePort(inputBuffer, requestUri);
  }

  if (AWS_URI_PARSING_ABS_PATH & _state) {
    return parseAbsPath(inputBuffer, requestUri);
  }

  if (AWS_URI_PARSING_QUERY & _state) {
    return parseQuery(inputBuffer, requestUri);
  }

  if (AWS_URI_PARSING_FRAGMENT & _state) {
    return parseFragment(inputBuffer, requestUri);
  }

  if (AWS_URI_PARSING_NON_HTTP_URI & _state) {
    return parseNonHttpUri(inputBuffer, requestUri);
  }

  return ESF_INVALID_STATE;
}

ESFError AWSHttpRequestUriParser::parseAsterisk(ESFBuffer *inputBuffer,
                                                AWSHttpRequestUri *requestUri) {
  ESF_ASSERT(AWS_URI_PARSING_ASTERISK & _state);

  if (false == inputBuffer->isReadable()) {
    return ESF_AGAIN;
  }

  if ('*' != inputBuffer->getNext()) {
    return AWS_HTTP_BAD_REQUEST_URI_ASTERISK;
  }

  if (false == inputBuffer->isReadable()) {
    inputBuffer->readReset();

    return ESF_AGAIN;
  }

  if (false == AWSHttpUtil::IsSpace(inputBuffer->getNext())) {
    return AWS_HTTP_BAD_REQUEST_URI_ASTERISK;
  }

  requestUri->setType(AWSHttpRequestUri::AWS_URI_ASTERISK);

  _state &= ~AWS_URI_PARSING_ASTERISK;
  _state |= AWS_URI_PARSE_COMPLETE;

  return ESF_SUCCESS;
}

ESFError AWSHttpRequestUriParser::parseAbsPath(ESFBuffer *inputBuffer,
                                               AWSHttpRequestUri *requestUri) {
  // abs_path      = "/"  path_segments
  // path_segments = segment *( "/" segment )
  // segment       = *pchar *( ";" param )
  // param         = *pchar

  ESF_ASSERT(AWS_URI_PARSING_ABS_PATH & _state);

  unsigned char octet;

  if (0 == _workingBuffer->getWritePosition()) {
    if (false == inputBuffer->isReadable()) {
      return ESF_AGAIN;
    }

    if (false == _workingBuffer->isWritable()) {
      return ESF_OVERFLOW;
    }

    octet = inputBuffer->getNext();

    if ('/' != octet) {
      return AWS_HTTP_BAD_REQUEST_URI_ABS_PATH;
    }

    _workingBuffer->putNext('/');
  }

  // ESFError error;

  while (true) {
    if (false == inputBuffer->isReadable()) {
      return ESF_AGAIN;
    }

    if (false == _workingBuffer->isWritable()) {
      return ESF_OVERFLOW;
    }

    octet = inputBuffer->getNext();

    /** DO NOT DECODE - a full parse of the absolute path is required to do this
     *  properly:

    if ('%' == octet)
    {
        inputBuffer->setReadPosition(inputBuffer->getReadPosition() - 1);

        error = AWSHttpUtil::DecodeEscape(inputBuffer, &octet);

        if (ESF_AGAIN == error)
        {
            return ESF_AGAIN;
        }

        if (ESF_SUCCESS != error)
        {
            return AWS_HTTP_BAD_REQUEST_URI_ABS_PATH;
        }

        _workingBuffer->putNext(octet);
        continue;
    } */

    if (AWSHttpUtil::IsPchar(octet)) {
      _workingBuffer->putNext(octet);
      continue;
    }

    if ('/' == octet || ';' == octet) {
      _workingBuffer->putNext(octet);
      continue;
    }

    if ('?' == octet) {
      requestUri->setAbsPath(_workingBuffer->duplicate(_allocator));

      if (0 == requestUri->getAbsPath()) {
        return ESF_OUT_OF_MEMORY;
      }

      inputBuffer->readMark();
      _workingBuffer->clear();

      _state &= ~AWS_URI_PARSING_ABS_PATH;
      _state |= AWS_URI_PARSING_QUERY;

      return parseQuery(inputBuffer, requestUri);
    }

    if ('#' == octet) {
      requestUri->setAbsPath(_workingBuffer->duplicate(_allocator));

      if (0 == requestUri->getAbsPath()) {
        return ESF_OUT_OF_MEMORY;
      }

      inputBuffer->readMark();
      _workingBuffer->clear();

      _state &= ~AWS_URI_PARSING_ABS_PATH;
      _state |= AWS_URI_PARSING_FRAGMENT;

      return parseFragment(inputBuffer, requestUri);
    }

    if (AWSHttpUtil::IsSpace(octet)) {
      requestUri->setAbsPath(_workingBuffer->duplicate(_allocator));

      if (0 == requestUri->getAbsPath()) {
        return ESF_OUT_OF_MEMORY;
      }

      inputBuffer->readMark();
      _workingBuffer->clear();

      _state &= ~AWS_URI_PARSING_ABS_PATH;
      _state |= AWS_URI_PARSE_COMPLETE;

      return ESF_SUCCESS;
    }

    return AWS_HTTP_BAD_REQUEST_URI_ABS_PATH;
  }
}

ESFError AWSHttpRequestUriParser::parseQuery(ESFBuffer *inputBuffer,
                                             AWSHttpRequestUri *requestUri) {
  // query         = *uric

  ESF_ASSERT(AWS_URI_PARSING_QUERY & _state);

  unsigned char octet;
  // ESFError error;

  while (true) {
    if (false == inputBuffer->isReadable()) {
      return ESF_AGAIN;
    }

    if (false == _workingBuffer->isWritable()) {
      return ESF_OVERFLOW;
    }

    octet = inputBuffer->getNext();

    /* DO NOT DECODE - A full parse of the query string parameters is
     * required to do this properly:

    if ('%' == octet)
    {
        inputBuffer->setReadPosition(inputBuffer->getReadPosition() - 1);

        error = AWSHttpUtil::DecodeEscape(inputBuffer, &octet);

        if (ESF_AGAIN == error)
        {
            return ESF_AGAIN;
        }

        if (ESF_SUCCESS != error)
        {
            return AWS_HTTP_BAD_REQUEST_URI_QUERY;
        }

        _workingBuffer->putNext(octet);
        continue;
    }*/

    if (AWSHttpUtil::IsUric(octet)) {
      _workingBuffer->putNext(octet);
      continue;
    }

    if ('#' == octet) {
      requestUri->setQuery(_workingBuffer->duplicate(_allocator));

      if (0 == requestUri->getQuery()) {
        return ESF_OUT_OF_MEMORY;
      }

      inputBuffer->readMark();
      _workingBuffer->clear();

      _state &= ~AWS_URI_PARSING_QUERY;
      _state |= AWS_URI_PARSING_FRAGMENT;

      return parseFragment(inputBuffer, requestUri);
    }

    if (AWSHttpUtil::IsSpace(octet)) {
      requestUri->setQuery(_workingBuffer->duplicate(_allocator));

      if (0 == requestUri->getQuery()) {
        return ESF_OUT_OF_MEMORY;
      }

      inputBuffer->readMark();
      _workingBuffer->clear();

      _state &= ~AWS_URI_PARSING_QUERY;
      _state |= AWS_URI_PARSE_COMPLETE;

      return ESF_SUCCESS;
    }

    return AWS_HTTP_BAD_REQUEST_URI_QUERY;
  }
}

ESFError AWSHttpRequestUriParser::parseFragment(ESFBuffer *inputBuffer,
                                                AWSHttpRequestUri *requestUri) {
  // fragment     = *uric

  ESF_ASSERT(AWS_URI_PARSING_FRAGMENT & _state);

  unsigned char octet;
  // ESFError error;

  while (true) {
    if (false == inputBuffer->isReadable()) {
      return ESF_AGAIN;
    }

    if (false == _workingBuffer->isWritable()) {
      return ESF_OVERFLOW;
    }

    octet = inputBuffer->getNext();

    /* DO NOT DECODE - interpretation of the fragment is up to the application

    if ('%' == octet)
    {
        inputBuffer->setReadPosition(inputBuffer->getReadPosition() - 1);

        error = AWSHttpUtil::DecodeEscape(inputBuffer, &octet);

        if (ESF_AGAIN == error)
        {
            return ESF_AGAIN;
        }

        if (ESF_SUCCESS != error)
        {
            return AWS_HTTP_BAD_REQUEST_URI_QUERY;
        }

        _workingBuffer->putNext(octet);
        continue;
    }*/

    if (AWSHttpUtil::IsUric(octet)) {
      _workingBuffer->putNext(octet);
      continue;
    }

    if (AWSHttpUtil::IsSpace(octet)) {
      requestUri->setFragment(_workingBuffer->duplicate(_allocator));

      if (0 == requestUri->getFragment()) {
        return ESF_OUT_OF_MEMORY;
      }

      inputBuffer->readMark();
      _workingBuffer->clear();

      _state &= ~AWS_URI_PARSING_FRAGMENT;
      _state |= AWS_URI_PARSE_COMPLETE;

      return ESF_SUCCESS;
    }

    return AWS_HTTP_BAD_REQUEST_URI_FRAGMENT;
  }
}

ESFError AWSHttpRequestUriParser::parseScheme(ESFBuffer *inputBuffer,
                                              AWSHttpRequestUri *requestUri) {
  // http_URL       = "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]
  // absoluteURI   = scheme ":" ( hier_part | opaque_part )
  // scheme        = alpha *( alpha | digit | "+" | "-" | "." )

  ESF_ASSERT(AWS_URI_PARSING_SCHEME & _state);

  unsigned char octet;

  if (0 == _workingBuffer->getWritePosition()) {
    if (false == inputBuffer->isReadable()) {
      return ESF_AGAIN;
    }

    if (false == _workingBuffer->isWritable()) {
      return ESF_OVERFLOW;
    }

    octet = inputBuffer->getNext();

    if (false == AWSHttpUtil::IsAlpha(octet)) {
      return AWS_HTTP_BAD_REQUEST_URI_SCHEME;
    }

    _workingBuffer->putNext(octet);
  }

  while (true) {
    if (false == inputBuffer->isReadable()) {
      return ESF_AGAIN;
    }

    if (false == _workingBuffer->isWritable()) {
      return ESF_OVERFLOW;
    }

    octet = inputBuffer->getNext();

    if (AWSHttpUtil::IsAlphaNum(octet)) {
      _workingBuffer->putNext(octet);
      continue;
    }

    switch (octet) {
      case '+':
      case '=':
      case '.':

        _workingBuffer->putNext(octet);
        break;

      case ':':

        inputBuffer->readMark();

        _state &= ~AWS_URI_PARSING_SCHEME;

        if (_workingBuffer->match(HTTP)) {
          _state |= AWS_URI_SKIPPING_FWD_SLASHES;

          requestUri->setType(AWSHttpRequestUri::AWS_URI_HTTP);

          _workingBuffer->clear();

          return skipForwardSlashes(inputBuffer, requestUri);
        } else if (_workingBuffer->match(HTTPS)) {
          _state |= AWS_URI_SKIPPING_FWD_SLASHES;

          requestUri->setType(AWSHttpRequestUri::AWS_URI_HTTPS);

          _workingBuffer->clear();

          return skipForwardSlashes(inputBuffer, requestUri);
        } else {
          _state |= AWS_URI_PARSING_NON_HTTP_URI;

          requestUri->setType(AWSHttpRequestUri::AWS_URI_OTHER);

          _workingBuffer->putNext(':');

          return parseNonHttpUri(inputBuffer, requestUri);
        }

      default:

        return AWS_HTTP_BAD_REQUEST_URI_SCHEME;
    }
  }
}

ESFError AWSHttpRequestUriParser::skipForwardSlashes(
    ESFBuffer *inputBuffer, AWSHttpRequestUri *requestUri) {
  // Skips the "//" in ...
  // http_URL       = "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]

  ESF_ASSERT(AWS_URI_SKIPPING_FWD_SLASHES & _state);

  while (true) {
    if (false == inputBuffer->isReadable()) {
      return ESF_AGAIN;
    }

    if ('/' == inputBuffer->peekNext()) {
      inputBuffer->skipNext();
      continue;
    }

    _state &= ~AWS_URI_SKIPPING_FWD_SLASHES;
    _state |= AWS_URI_PARSING_HOST;

    return parseHost(inputBuffer, requestUri);
  }
}

ESFError AWSHttpRequestUriParser::parseHost(ESFBuffer *inputBuffer,
                                            AWSHttpRequestUri *requestUri) {
  // host          = hostname | IPv4address
  // hostname      = *( domainlabel "." ) toplabel [ "." ]
  // domainlabel   = alphanum | alphanum *( alphanum | "-" ) alphanum
  // toplabel      = alpha | alpha *( alphanum | "-" ) alphanum
  // IPv4address   = 1*digit "." 1*digit "." 1*digit "." 1*digit

  // todo whoops username and password may also be present here!  which spec
  // controls this? todo the parseHost code is too permissive

  ESF_ASSERT(AWS_URI_PARSING_HOST & _state);

  unsigned char octet;

  if (0 == _workingBuffer->getWritePosition()) {
    if (false == inputBuffer->isReadable()) {
      return ESF_AGAIN;
    }

    if (false == _workingBuffer->isWritable()) {
      return ESF_OVERFLOW;
    }

    octet = inputBuffer->getNext();

    if (false == AWSHttpUtil::IsAlphaNum(octet)) {
      return AWS_HTTP_BAD_REQUEST_URI_HOST;
    }

    _workingBuffer->putNext(octet);
  }

  while (true) {
    if (false == inputBuffer->isReadable()) {
      return ESF_AGAIN;
    }

    if (false == _workingBuffer->isWritable()) {
      return ESF_OVERFLOW;
    }

    octet = inputBuffer->getNext();

    if (AWSHttpUtil::IsAlphaNum(octet)) {
      _workingBuffer->putNext(octet);
      continue;
    }

    switch (octet) {
      case '-':
      case '.':

        _workingBuffer->putNext(octet);
        break;

      case ':':

        requestUri->setHost(_workingBuffer->duplicate(_allocator));

        if (0 == requestUri->getHost()) {
          return ESF_OUT_OF_MEMORY;
        }

        inputBuffer->readMark();
        _workingBuffer->clear();

        _state &= ~AWS_URI_PARSING_HOST;
        _state |= AWS_URI_PARSING_PORT;

        return parsePort(inputBuffer, requestUri);

      case ' ':
      case '\t':

        requestUri->setHost(_workingBuffer->duplicate(_allocator));

        if (0 == requestUri->getHost()) {
          return ESF_OUT_OF_MEMORY;
        }

        inputBuffer->readMark();
        _workingBuffer->clear();

        _state &= ~AWS_URI_PARSING_HOST;
        _state |= AWS_URI_PARSE_COMPLETE;

        requestUri->setAbsPath((const unsigned char *)"/");

        return ESF_SUCCESS;

      case '/':

        requestUri->setHost(_workingBuffer->duplicate(_allocator));

        if (0 == requestUri->getHost()) {
          return ESF_OUT_OF_MEMORY;
        }

        inputBuffer->setReadPosition(inputBuffer->getReadPosition() - 1);
        inputBuffer->readMark();
        _workingBuffer->clear();

        _state &= ~AWS_URI_PARSING_HOST;
        _state |= AWS_URI_PARSING_ABS_PATH;

        return parseAbsPath(inputBuffer, requestUri);

      case '?':

        requestUri->setHost(_workingBuffer->duplicate(_allocator));

        if (0 == requestUri->getHost()) {
          return ESF_OUT_OF_MEMORY;
        }

        inputBuffer->readMark();
        _workingBuffer->clear();

        _state &= ~AWS_URI_PARSING_HOST;
        _state |= AWS_URI_PARSING_QUERY;

        requestUri->setAbsPath((const unsigned char *)"/");

        return parseQuery(inputBuffer, requestUri);

      case '#':

        requestUri->setHost(_workingBuffer->duplicate(_allocator));

        if (0 == requestUri->getHost()) {
          return ESF_OUT_OF_MEMORY;
        }

        inputBuffer->readMark();
        _workingBuffer->clear();

        _state &= ~AWS_URI_PARSING_HOST;
        _state |= AWS_URI_PARSING_FRAGMENT;

        requestUri->setAbsPath((const unsigned char *)"/");

        return parseFragment(inputBuffer, requestUri);

      default:

        return AWS_HTTP_BAD_REQUEST_URI_HOST;
    }
  }
}

ESFError AWSHttpRequestUriParser::parsePort(ESFBuffer *inputBuffer,
                                            AWSHttpRequestUri *requestUri) {
  // port          = *digit

  ESF_ASSERT(AWS_URI_PARSING_PORT & _state);

  unsigned char octet;

  while (true) {
    if (false == inputBuffer->isReadable()) {
      return ESF_AGAIN;
    }

    octet = inputBuffer->getNext();

    if (AWSHttpUtil::IsDigit(octet)) {
      if (0 > requestUri->getPort()) {
        requestUri->setPort(0);
      }

      requestUri->setPort(requestUri->getPort() * 10 + (octet - '0'));

      if (65536 <= requestUri->getPort()) {
        return AWS_HTTP_BAD_REQUEST_URI_PORT;
      }

      continue;
    }

    if (AWSHttpUtil::IsSpace(octet)) {
      if (0 > requestUri->getPort()) {
        return AWS_HTTP_BAD_REQUEST_URI_PORT;
      }

      inputBuffer->readMark();
      _workingBuffer->clear();

      _state &= ~AWS_URI_PARSING_PORT;
      _state |= AWS_URI_PARSE_COMPLETE;

      requestUri->setAbsPath((const unsigned char *)"/");

      return ESF_SUCCESS;
    }

    if ('/' == octet) {
      if (0 > requestUri->getPort()) {
        return AWS_HTTP_BAD_REQUEST_URI_PORT;
      }

      _state &= ~AWS_URI_PARSING_PORT;
      _state |= AWS_URI_PARSING_ABS_PATH;

      inputBuffer->setReadPosition(inputBuffer->getReadPosition() - 1);
      inputBuffer->readMark();
      _workingBuffer->clear();

      return parseAbsPath(inputBuffer, requestUri);
    }

    if ('?' == octet) {
      if (0 > requestUri->getPort()) {
        return AWS_HTTP_BAD_REQUEST_URI_PORT;
      }

      _state &= ~AWS_URI_PARSING_PORT;
      _state |= AWS_URI_PARSING_QUERY;

      inputBuffer->readMark();
      _workingBuffer->clear();

      requestUri->setAbsPath((const unsigned char *)"/");

      return parseQuery(inputBuffer, requestUri);
    }

    if ('#' == octet) {
      if (0 > requestUri->getPort()) {
        return AWS_HTTP_BAD_REQUEST_URI_PORT;
      }

      _state &= ~AWS_URI_PARSING_PORT;
      _state |= AWS_URI_PARSING_FRAGMENT;

      inputBuffer->readMark();
      _workingBuffer->clear();

      requestUri->setAbsPath((const unsigned char *)"/");

      return parseFragment(inputBuffer, requestUri);
    }

    return AWS_HTTP_BAD_REQUEST_URI_PORT;
  }
}

ESFError AWSHttpRequestUriParser::parseNonHttpUri(
    ESFBuffer *inputBuffer, AWSHttpRequestUri *requestUri) {
  // absoluteURI   = scheme ":" ( hier_part | opaque_part )
  // hier_part     = ( net_path | abs_path ) [ "?" query ]
  // net_path      = "//" authority [ abs_path ]
  // abs_path      = "/"  path_segments
  // opaque_part   = uric_no_slash *uric
  // uric_no_slash = unreserved | escaped | ";" | "?" | ":" | "@" | "&" | "=" |
  // "+" | "$" | ","

  ESF_ASSERT(AWS_URI_PARSING_NON_HTTP_URI & _state);

  unsigned char octet;

  while (true) {
    if (false == inputBuffer->isReadable()) {
      return ESF_AGAIN;
    }

    if (false == _workingBuffer->isWritable()) {
      return ESF_OVERFLOW;
    }

    octet = inputBuffer->getNext();

    if (AWSHttpUtil::IsSpace(octet)) {
      requestUri->setOther(_workingBuffer->duplicate(_allocator));

      if (0 == requestUri->getOther()) {
        return ESF_OUT_OF_MEMORY;
      }

      inputBuffer->readMark();
      _workingBuffer->clear();

      _state &= ~AWS_URI_PARSING_NON_HTTP_URI;
      _state |= AWS_URI_PARSE_COMPLETE;

      return ESF_SUCCESS;
    }

    _workingBuffer->putNext(octet);
  }
}
