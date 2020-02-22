/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_REQUEST_URI_FORMATTER_H
#include <AWSHttpRequestUriFormatter.h>
#endif

#ifndef AWS_HTTP_ERROR_H
#include <AWSHttpError.h>
#endif

#ifndef AWS_HTTP_UTIL_H
#include <AWSHttpUtil.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#define AWS_URI_FORMATTING_ASTERISK (1 << 0)
#define AWS_URI_FORMATTING_SCHEME (1 << 1)
#define AWS_URI_FORMATTING_HOST (1 << 2)
#define AWS_URI_FORMATTING_PORT (1 << 3)
#define AWS_URI_FORMATTING_ABS_PATH (1 << 4)
#define AWS_URI_FORMATTING_QUERY (1 << 5)
#define AWS_URI_FORMATTING_FRAGMENT (1 << 6)
#define AWS_URI_FORMAT_COMPLETE (1 << 7)
#define AWS_URI_FORMATTING_NON_HTTP_URI (1 << 8)

AWSHttpRequestUriFormatter::AWSHttpRequestUriFormatter() : _state(0x00) {}

AWSHttpRequestUriFormatter::~AWSHttpRequestUriFormatter() {}

void AWSHttpRequestUriFormatter::reset() { _state = 0x00; }

ESFError AWSHttpRequestUriFormatter::format(
    ESFBuffer *outputBuffer, const AWSHttpRequestUri *requestUri) {
  // Request-URI   = "*" | absoluteURI | abs_path [ "?" query ] | authority

  if (AWS_URI_FORMAT_COMPLETE & _state) {
    return ESF_INVALID_STATE;
  }

  if (0x00 == _state) {
    if (false == outputBuffer->isWritable()) {
      return ESF_AGAIN;
    }

    switch (requestUri->getType()) {
      case AWSHttpRequestUri::AWS_URI_ASTERISK:

        AWSHttpUtil::Start(&_state, outputBuffer, AWS_URI_FORMATTING_ASTERISK);

        break;

      case AWSHttpRequestUri::AWS_URI_HTTP:
      case AWSHttpRequestUri::AWS_URI_HTTPS:

        if (requestUri->getHost() && 0 != requestUri->getHost()[0]) {
          AWSHttpUtil::Start(&_state, outputBuffer, AWS_URI_FORMATTING_SCHEME);
        } else {
          AWSHttpUtil::Start(&_state, outputBuffer,
                             AWS_URI_FORMATTING_ABS_PATH);
        }

        break;

      case AWSHttpRequestUri::AWS_URI_OTHER:

        AWSHttpUtil::Start(&_state, outputBuffer,
                           AWS_URI_FORMATTING_NON_HTTP_URI);

        break;

      default:

        return AWSHttpUtil::Rollback(outputBuffer, ESF_INVALID_ARGUMENT);
    }
  }

  ESFError error;

  if (AWS_URI_FORMATTING_ASTERISK & _state) {
    return formatAsterisk(outputBuffer, requestUri);
  }

  if (AWS_URI_FORMATTING_NON_HTTP_URI & _state) {
    return formatNonHttpUri(outputBuffer, requestUri);
  }

  if (AWS_URI_FORMATTING_SCHEME & _state) {
    error = formatScheme(outputBuffer, requestUri);

    if (ESF_SUCCESS != error) {
      return error;
    }
  }

  if (AWS_URI_FORMATTING_HOST & _state) {
    error = formatHost(outputBuffer, requestUri);

    if (ESF_SUCCESS != error) {
      return error;
    }
  }

  if (AWS_URI_FORMATTING_PORT & _state) {
    error = formatPort(outputBuffer, requestUri);

    if (ESF_SUCCESS != error) {
      return error;
    }
  }

  if (AWS_URI_FORMATTING_ABS_PATH & _state) {
    error = formatAbsPath(outputBuffer, requestUri);

    if (ESF_SUCCESS != error) {
      return error;
    }
  }

  if (AWS_URI_FORMATTING_QUERY & _state) {
    error = formatQuery(outputBuffer, requestUri);

    if (ESF_SUCCESS != error) {
      return error;
    }
  }

  if (AWS_URI_FORMATTING_FRAGMENT & _state) {
    return formatFragment(outputBuffer, requestUri);
  }

  return AWSHttpUtil::Rollback(outputBuffer, ESF_INVALID_STATE);
}

ESFError AWSHttpRequestUriFormatter::formatAsterisk(
    ESFBuffer *outputBuffer, const AWSHttpRequestUri *requestUri) {
  // "*"

  ESF_ASSERT(AWS_URI_FORMATTING_ASTERISK & _state);

  if (false == outputBuffer->getWritable()) {
    return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
  }

  outputBuffer->putNext('*');

  return AWSHttpUtil::Transition(&_state, outputBuffer,
                                 AWS_URI_FORMATTING_ASTERISK,
                                 AWS_URI_FORMAT_COMPLETE);
}

ESFError AWSHttpRequestUriFormatter::formatAbsPath(
    ESFBuffer *outputBuffer, const AWSHttpRequestUri *requestUri) {
  // abs_path      = "/"  path_segments
  // path_segments = segment *( "/" segment )
  // segment       = *pchar *( ";" param )
  // param         = *pchar

  ESF_ASSERT(AWS_URI_FORMATTING_ABS_PATH & _state);

  const unsigned char *p = 0 == requestUri->getAbsPath()
                               ? (const unsigned char *)"/"
                               : requestUri->getAbsPath();

  if ('/' != *p) {
    if (false == outputBuffer->isWritable()) {
      return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
    }

    outputBuffer->putNext('/');
  }

  ESFError error;

  for (; *p; ++p) {
    if (false == outputBuffer->isWritable()) {
      return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
    }

    if (AWSHttpUtil::IsPchar(*p)) {
      outputBuffer->putNext(*p);
      continue;
    }

    if ('/' == *p || ';' == *p) {
      outputBuffer->putNext(*p);
      continue;
    }

    error = AWSHttpUtil::EncodeEscape(outputBuffer, *p);

    if (ESF_SUCCESS != error) {
      return AWSHttpUtil::Rollback(outputBuffer, error);
    }
  }

  return AWSHttpUtil::Transition(&_state, outputBuffer,
                                 AWS_URI_FORMATTING_ABS_PATH,
                                 AWS_URI_FORMATTING_QUERY);
}

ESFError AWSHttpRequestUriFormatter::formatQuery(
    ESFBuffer *outputBuffer, const AWSHttpRequestUri *requestUri) {
  // query         = *uric

  ESF_ASSERT(AWS_URI_FORMATTING_QUERY & _state);

  const unsigned char *p = requestUri->getQuery();

  if (!p || 0 == *p) {
    return AWSHttpUtil::Transition(&_state, outputBuffer,
                                   AWS_URI_FORMATTING_QUERY,
                                   AWS_URI_FORMATTING_FRAGMENT);
  }

  if (false == outputBuffer->isWritable()) {
    return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
  }

  outputBuffer->putNext('?');

  ESFError error;

  for (; *p; ++p) {
    if (false == outputBuffer->isWritable()) {
      return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
    }

    if (AWSHttpUtil::IsUric(*p)) {
      outputBuffer->putNext(*p);
      continue;
    }

    error = AWSHttpUtil::EncodeEscape(outputBuffer, *p);

    if (ESF_SUCCESS != error) {
      return AWSHttpUtil::Rollback(outputBuffer, error);
    }
  }

  return AWSHttpUtil::Transition(&_state, outputBuffer,
                                 AWS_URI_FORMATTING_QUERY,
                                 AWS_URI_FORMATTING_FRAGMENT);
}

ESFError AWSHttpRequestUriFormatter::formatFragment(
    ESFBuffer *outputBuffer, const AWSHttpRequestUri *requestUri) {
  // fragment         = *uric

  ESF_ASSERT(AWS_URI_FORMATTING_FRAGMENT & _state);

  const unsigned char *p = requestUri->getFragment();

  if (!p || 0 == *p) {
    return AWSHttpUtil::Transition(&_state, outputBuffer,
                                   AWS_URI_FORMATTING_FRAGMENT,
                                   AWS_URI_FORMAT_COMPLETE);
  }

  if (false == outputBuffer->isWritable()) {
    return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
  }

  outputBuffer->putNext('#');

  ESFError error;

  for (; *p; ++p) {
    if (false == outputBuffer->isWritable()) {
      return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
    }

    if (AWSHttpUtil::IsUric(*p)) {
      outputBuffer->putNext(*p);
      continue;
    }

    error = AWSHttpUtil::EncodeEscape(outputBuffer, *p);

    if (ESF_SUCCESS != error) {
      return AWSHttpUtil::Rollback(outputBuffer, error);
    }
  }

  return AWSHttpUtil::Transition(&_state, outputBuffer,
                                 AWS_URI_FORMATTING_FRAGMENT,
                                 AWS_URI_FORMAT_COMPLETE);
}

ESFError AWSHttpRequestUriFormatter::formatScheme(
    ESFBuffer *outputBuffer, const AWSHttpRequestUri *requestUri) {
  // http_URL       = "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]
  // absoluteURI   = scheme ":" ( hier_part | opaque_part )
  // scheme        = alpha *( alpha | digit | "+" | "-" | "." )

  ESF_ASSERT(AWS_URI_FORMATTING_SCHEME & _state);
  ESF_ASSERT(AWSHttpRequestUri::AWS_URI_ASTERISK != requestUri->getType());

  const char *p = 0;

  switch (requestUri->getType()) {
    case AWSHttpRequestUri::AWS_URI_ASTERISK:

      return AWSHttpUtil::Rollback(outputBuffer, ESF_INVALID_STATE);

    case AWSHttpRequestUri::AWS_URI_HTTP:

      p = "http://";

      break;

    case AWSHttpRequestUri::AWS_URI_HTTPS:

      p = "https://";

      break;

    case AWSHttpRequestUri::AWS_URI_OTHER:

      return AWSHttpUtil::Transition(&_state, outputBuffer,
                                     AWS_URI_FORMATTING_SCHEME,
                                     AWS_URI_FORMATTING_HOST);

    default:

      return AWSHttpUtil::Rollback(outputBuffer,
                                   AWS_HTTP_BAD_REQUEST_URI_SCHEME);
  }

  for (; *p; ++p) {
    if (false == outputBuffer->isWritable()) {
      return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
    }

    outputBuffer->putNext((unsigned char)*p);
  }

  return AWSHttpUtil::Transition(&_state, outputBuffer,
                                 AWS_URI_FORMATTING_SCHEME,
                                 AWS_URI_FORMATTING_HOST);
}

ESFError AWSHttpRequestUriFormatter::formatHost(
    ESFBuffer *outputBuffer, const AWSHttpRequestUri *requestUri) {
  // host          = hostname | IPv4address
  // hostname      = *( domainlabel "." ) toplabel [ "." ]
  // domainlabel   = alphanum | alphanum *( alphanum | "-" ) alphanum
  // toplabel      = alpha | alpha *( alphanum | "-" ) alphanum
  // IPv4address   = 1*digit "." 1*digit "." 1*digit "." 1*digit

  ESF_ASSERT(AWS_URI_FORMATTING_HOST & _state);
  ESF_ASSERT(requestUri->getHost());
  ESF_ASSERT(0 != requestUri->getHost()[0]);
  ESF_ASSERT(AWSHttpRequestUri::AWS_URI_HTTP == requestUri->getType() ||
             AWSHttpRequestUri::AWS_URI_HTTPS == requestUri->getType());

  for (const unsigned char *p = requestUri->getHost(); *p; ++p) {
    if (false == outputBuffer->isWritable()) {
      return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
    }

    outputBuffer->putNext(*p);  // TODO add validation
  }

  return AWSHttpUtil::Transition(&_state, outputBuffer, AWS_URI_FORMATTING_HOST,
                                 AWS_URI_FORMATTING_PORT);
}

ESFError AWSHttpRequestUriFormatter::formatPort(
    ESFBuffer *outputBuffer, const AWSHttpRequestUri *requestUri) {
  // port          = *digit

  ESF_ASSERT(AWS_URI_FORMATTING_PORT & _state);
  ESF_ASSERT(requestUri->getHost());
  ESF_ASSERT(0 != requestUri->getHost()[0]);
  ESF_ASSERT(65536 > requestUri->getPort());
  ESF_ASSERT(AWSHttpRequestUri::AWS_URI_HTTP == requestUri->getType() ||
             AWSHttpRequestUri::AWS_URI_HTTPS == requestUri->getType());

  int port = requestUri->getPort();

  switch (requestUri->getType()) {
    case AWSHttpRequestUri::AWS_URI_HTTP:

      if (0 > port || 80 == port) {
        return AWSHttpUtil::Transition(&_state, outputBuffer,
                                       AWS_URI_FORMATTING_PORT,
                                       AWS_URI_FORMATTING_ABS_PATH);
      }

      break;

    case AWSHttpRequestUri::AWS_URI_HTTPS:

      if (0 > port || 443 == port) {
        return AWSHttpUtil::Transition(&_state, outputBuffer,
                                       AWS_URI_FORMATTING_PORT,
                                       AWS_URI_FORMATTING_ABS_PATH);
      }

      break;

    default:

      return AWSHttpUtil::Rollback(outputBuffer, AWS_HTTP_BAD_REQUEST_URI_PORT);
  }

  ESF_ASSERT(0 <= requestUri->getPort());

  if (65536 <= port) {
    return AWSHttpUtil::Rollback(outputBuffer, AWS_HTTP_BAD_REQUEST_URI_PORT);
  }

  if (false == outputBuffer->isWritable()) {
    return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
  }

  outputBuffer->putNext(':');

  ESFError error = AWSHttpUtil::FormatInteger(outputBuffer, port, 10);

  if (ESF_SUCCESS != error) {
    return AWSHttpUtil::Rollback(outputBuffer, error);
  }

  return AWSHttpUtil::Transition(&_state, outputBuffer, AWS_URI_FORMATTING_PORT,
                                 AWS_URI_FORMATTING_ABS_PATH);
}

ESFError AWSHttpRequestUriFormatter::formatNonHttpUri(
    ESFBuffer *outputBuffer, const AWSHttpRequestUri *requestUri) {
  // absoluteURI   = scheme ":" ( hier_part | opaque_part )
  // hier_part     = ( net_path | abs_path ) [ "?" query ]
  // net_path      = "//" authority [ abs_path ]
  // abs_path      = "/"  path_segments
  // opaque_part   = uric_no_slash *uric
  // uric_no_slash = unreserved | escaped | ";" | "?" | ":" | "@" | "&" | "=" |
  // "+" | "$" | ","

  ESF_ASSERT(AWS_URI_FORMATTING_NON_HTTP_URI & _state);
  ESF_ASSERT(AWSHttpRequestUri::AWS_URI_OTHER == requestUri->getType());

  if (0 == requestUri->getOther() || 0 == requestUri->getOther()[0]) {
    return AWSHttpUtil::Rollback(outputBuffer, ESF_INVALID_ARGUMENT);
  }

  for (const unsigned char *p = requestUri->getOther(); *p; ++p) {
    if (false == outputBuffer->isWritable()) {
      return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
    }

    outputBuffer->putNext(*p);
  }

  return AWSHttpUtil::Transition(&_state, outputBuffer,
                                 AWS_URI_FORMATTING_NON_HTTP_URI,
                                 AWS_URI_FORMAT_COMPLETE);
}
