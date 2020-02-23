#ifndef ES_HTTP_UTIL_H
#include <ESHttpUtil.h>
#endif

#ifndef ES_HTTP_ERROR_H
#include <ESHttpError.h>
#endif

namespace ES {

/**
 * This is an array of bitmasks.  The index into this array is:
 * <p/>
 * The hexadecimal ASCII set:
 * <p/>
 * 00 nul   01 soh   02 stx   03 etx   04 eot   05 enq   06 ack   07 bel
 * 08 bs    09 ht    0a nl    0b vt    0c np    0d cr    0e so    0f si
 * 10 dle   11 dc1   12 dc2   13 dc3   14 dc4   15 nak   16 syn   17 etb
 * 18 can   19 em    1a sub   1b esc   1c fs    1d gs    1e rs    1f us
 * 20 sp    21  !    22  "    23  #    24  $    25  %    26  &    27  '
 * 28  (    29  )    2a  *    2b  +    2c  ,    2d  -    2e  .    2f  /
 * 30  0    31  1    32  2    33  3    34  4    35  5    36  6    37  7
 * 38  8    39  9    3a  :    3b  ;    3c  <    3d  =    3e  >    3f  ?
 * 40  @    41  A    42  B    43  C    44  D    45  E    46  F    47  G
 * 48  H    49  I    4a  J    4b  K    4c  L    4d  M    4e  N    4f  O
 * 50  P    51  Q    52  R    53  S    54  T    55  U    56  V    57  W
 * 58  X    59  Y    5a  Z    5b  [    5c  \    5d  ]    5e  ^    5f  _
 * 60  `    61  a    62  b    63  c    64  d    65  e    66  f    67  g
 * 68  h    69  i    6a  j    6b  k    6c  l    6d  m    6e  n    6f  o
 * 70  p    71  q    72  r    73  s    74  t    75  u    76  v    77  w
 * 78  x    79  y    7a  z    7b  {    7c  |    7d  }    7e  ~    7f del
 */
ESB::UInt16 HttpUtil::_Bitmasks[] = {
    //           x0      x1      x2      x3      x4      x5      x6      x7 x8
    //           x9      xA      xB      xC      xD      xE      xF
    /* 0x */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000,          0x0000, 0x0000, 0x0300, 0x0000, 0x0000,
    0x0000,          0x0000, 0x0000, 0x0000,
    /* 1x */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000,          0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000,          0x0000, 0x0000, 0x0000,
    /* 2x */ 0x0300, 0x0508, 0x0300, 0x0500, 0x05C0, 0x0520,
    0x05C0,          0x0508, 0x0308, 0x0308, 0x0508, 0x05C0,
    0x03C0,          0x0508, 0x0508, 0x0380,
    /* 3x */ 0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504,
    0x0504,          0x0504, 0x0504, 0x0504, 0x03C0, 0x0380,
    0x0300,          0x03C0, 0x0300, 0x0380,
    /* 4x */ 0x03C0, 0x0512, 0x0512, 0x0512, 0x0512, 0x0512,
    0x0512,          0x0502, 0x0502, 0x0502, 0x0502, 0x0502,
    0x0502,          0x0502, 0x0502, 0x0502,
    /* 5x */ 0x0502, 0x0502, 0x0502, 0x0502, 0x0502, 0x0502,
    0x0502,          0x0502, 0x0502, 0x0502, 0x0502, 0x0300,
    0x0300,          0x0300, 0x0500, 0x0508,
    /* 6x */ 0x0500, 0x0511, 0x0511, 0x0511, 0x0511, 0x0511,
    0x0511,          0x0501, 0x0501, 0x0501, 0x0501, 0x0501,
    0x0501,          0x0501, 0x0501, 0x0501,
    /* 7x */ 0x0501, 0x0501, 0x0501, 0x0501, 0x0501, 0x0501,
    0x0501,          0x0501, 0x0501, 0x0501, 0x0501, 0x0300,
    0x0500,          0x0300, 0x0508, 0x0000,
    /* 8x */ 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100,
    /* 9x */ 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100,
    /* Ax */ 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100,
    /* Bx */ 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100,
    /* Cx */ 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100,
    /* Dx */ 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100,
    /* Ex */ 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100,
    /* Fx */ 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100,          0x0100, 0x0100, 0x0100};

unsigned char *HttpUtil::Duplicate(ESB::Allocator *allocator,
                                   const unsigned char *value) {
  if (!value || !allocator) {
    return 0;
  }

  int length = strlen((const char *)value);

  unsigned char *duplicate = (unsigned char *)allocator->allocate(length + 1);

  if (!duplicate) {
    return 0;
  }

  memcpy(duplicate, value, length);
  duplicate[length] = 0;

  return duplicate;
}

ESB::Error HttpUtil::DecodeEscape(ESB::Buffer *buffer, unsigned char *value) {
  if (!value) {
    return ESB_NULL_POINTER;
  }

  if (3 > buffer->getReadable()) {
    return ESB_AGAIN;
  }

  unsigned char octet = buffer->getNext();

  if ('%' != octet) {
    return ESB_ILLEGAL_ENCODING;
  }

  octet = buffer->getNext();

  if (false == IsHex(octet)) {
    return ESB_ILLEGAL_ENCODING;
  }

  *value = 0;

  if (IsUpAlpha(octet)) {
    *value = (octet - 'A' + 10) * 16;
  } else if (IsLowAlpha(octet)) {
    *value = (octet - 'a' + 10) * 16;
  } else {
    *value = (octet - '0') * 16;
  }

  assert(buffer->isReadable());

  octet = buffer->getNext();

  if (false == IsHex(octet)) {
    return ESB_ILLEGAL_ENCODING;
  }

  if (IsUpAlpha(octet)) {
    *value += octet - 'A' + 10;
  } else if (IsLowAlpha(octet)) {
    *value += octet - 'a' + 10;
  } else {
    *value += octet - '0';
  }

  return ESB_SUCCESS;
}

ESB::Error HttpUtil::EncodeEscape(ESB::Buffer *buffer, unsigned char value) {
  if (!value) {
    return ESB_NULL_POINTER;
  }

  if (3 > buffer->getWritable()) {
    return ESB_AGAIN;
  }

  buffer->putNext('%');

  unsigned char octet = (0xF0 & value) >> 4;

  if (10 <= octet) {
    buffer->putNext('A' + (octet - 10));
  } else {
    buffer->putNext('0' + octet);
  }

  octet = 0x0F & value;

  if (10 <= octet) {
    buffer->putNext('A' + (octet - 10));
  } else {
    buffer->putNext('0' + octet);
  }

  return ESB_SUCCESS;
}

void HttpUtil::SkipSpaces(ESB::Buffer *buffer) {
  while (true) {
    if (false == buffer->isReadable()) {
      return;
    }

    switch (buffer->peekNext()) {
      case ' ':
      case '\t':

        buffer->skipNext();
        break;

      default:

        return;
    }
  }
}

void HttpUtil::SkipWhitespace(ESB::Buffer *buffer) {
  while (true) {
    if (false == buffer->isReadable()) {
      return;
    }

    switch (buffer->peekNext()) {
      case ' ':
      case '\t':
      case '\r':
      case '\n':

        buffer->skipNext();
        break;

      default:

        return;
    }
  }
}

ESB::Error HttpUtil::SkipLine(ESB::Buffer *buffer, int *bytesSkipped) {
  // The line terminator for message-header fields is the sequence CRLF.
  // However, we recommend that applications, when parsing such headers,
  // recognize a single LF as a line terminator and ignore the leading CR.

  int position = buffer->getReadPosition();
  bool foundCarriageReturn = false;
  unsigned char octet;

  *bytesSkipped = 0;

  while (true) {
    if (false == buffer->isReadable()) {
      buffer->setReadPosition(position);
      return ESB_AGAIN;
    }

    octet = buffer->getNext();

    switch (octet) {
      case '\r':

        if (foundCarriageReturn) {
          *bytesSkipped = *bytesSkipped + 1;
        } else {
          foundCarriageReturn = true;
        }

        break;

      case '\n':

        return ESB_SUCCESS;

      default:

        if (foundCarriageReturn) {
          *bytesSkipped = *bytesSkipped + 1;
          foundCarriageReturn = false;
        }

        *bytesSkipped = *bytesSkipped + 1;
    }
  }
}

ESB::Error HttpUtil::SkipLWS(ESB::Buffer *buffer, int depth) {
  // The line terminator for message-header fields is the sequence CRLF.
  // However, we recommend that applications, when parsing such headers,
  // recognize a single LF as a line terminator and ignore the leading CR.

  if (0 > depth) {
    return ESB_OVERFLOW;
  }

  unsigned int mark = buffer->getReadPosition();

  bool foundCRLF = false;

  while (true) {
    if (false == buffer->isReadable()) {
      buffer->setReadPosition(mark);
      return ESB_AGAIN;
    }

    unsigned char octet = buffer->getNext();

    switch (octet) {
      case '\r':

        if (foundCRLF) {
          // CRLF followed by something not ' ' or \t ==> end of line

          buffer->setReadPosition(buffer->getReadPosition() - 1);

          return ESB_SUCCESS;
        }

        break;

      case '\n':

        if (foundCRLF) {
          // CRLF followed by something not ' ' or \t ==> end of line

          buffer->setReadPosition(buffer->getReadPosition() - 1);

          return ESB_SUCCESS;
        }

        foundCRLF = true;

        break;

      case ' ':
      case '\t':

        if (foundCRLF) {
          // We have a line continuation - skip the CRLF and keep going...

          foundCRLF = false;
        }

        // mark = buffer->getReadPosition();

        break;

      default:

        if (foundCRLF) {
          // CRLF followed by something not ' ' or \t ==> end of line

          buffer->setReadPosition(buffer->getReadPosition() - 1);

          return ESB_SUCCESS;
        }

        // We found field data

        buffer->setReadPosition(buffer->getReadPosition() - 1);

        return ESB_INPROGRESS;
    }
  }
}

bool HttpUtil::IsSpace(unsigned char octet) {
  switch (octet) {
    case ' ':
    case '\t':
      return true;

    default:
      return false;
  }
}

bool HttpUtil::IsLWS(unsigned char octet) {
  switch (octet) {
    case '\r':
    case '\n':
    case ' ':
    case '\t':
      return true;

    default:
      return false;
  }
}

bool HttpUtil::IsMatch(ESB::Buffer *buffer, const unsigned char *literal) {
  unsigned int mark = buffer->getReadPosition();

  for (const unsigned char *p = literal; *p; ++p) {
    if (*p != buffer->getNext()) {
      buffer->setReadPosition(mark);

      return false;
    }
  }

  buffer->setReadPosition(mark);

  return true;
}

bool HttpUtil::EndsWith(const char *str, int strLength, const char *pattern,
                        int patternLength) {
  if (!str || !pattern) {
    return false;
  }

  if (0 >= strLength || 0 >= patternLength) {
    return false;
  }

  if (patternLength > strLength) {
    return false;
  }

  for (int i = patternLength - 1, j = strLength - 1; i >= 0; --i, --j) {
    if (pattern[i] != str[j]) {
      return false;
    }
  }

  return true;
}

ESB::Error HttpUtil::ParseInteger(unsigned const char *str,
                                  ESB::UInt64 *integer) {
  if (!str || !integer) {
    return ESB_NULL_POINTER;
  }

  *integer = 0;

  for (unsigned const char *p = str; *p; ++p) {
    if (false == IsDigit(*p)) {
      return ES_HTTP_BAD_INTEGER;
    }

    *integer = (*integer * 10) + (*p - '0');
  }

  return ESB_SUCCESS;
}

ESB::Error HttpUtil::FormatInteger(ESB::Buffer *buffer, ESB::UInt64 integer,
                                   int radix) {
  if (!buffer) {
    return ESB_NULL_POINTER;
  }

  if (2 > radix || 16 < radix) {
    return ESB_INVALID_ARGUMENT;
  }

  if (0 == integer) {
    if (false == buffer->isWritable()) {
      return ESB_AGAIN;
    }

    buffer->putNext('0');

    return ESB_SUCCESS;
  }

  int tmp = integer;
  int magnitude = 1;
  unsigned int length = 0;

  while (tmp > 0) {
    magnitude *= radix;
    tmp /= radix;
    ++length;
  }

  if (length > buffer->getWritable()) {
    return ESB_AGAIN;
  }

  int digit = 0;

  for (; magnitude >= radix; magnitude /= radix) {
    assert(buffer->isWritable());

    digit = (integer % magnitude) / (magnitude / radix);

    if (digit < 10) {
      buffer->putNext('0' + digit);
    } else {
      buffer->putNext('a' + digit - 10);
    }
  }

  return ESB_SUCCESS;
}

}  // namespace ES
