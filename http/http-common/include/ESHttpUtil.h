#ifndef ES_HTTP_UTIL_H
#define ES_HTTP_UTIL_H

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ES {

class HttpUtil {
 public:
  static unsigned char *Duplicate(ESB::Allocator *allocator, const unsigned char *value);

  static ESB::Error DecodeEscape(ESB::Buffer *buffer, unsigned char *value);

  static ESB::Error EncodeEscape(ESB::Buffer *buffer, unsigned char value);

  /**
   * Skips ' ' & '\t' only
   *
   * @param buffer
   */
  static void SkipSpaces(ESB::Buffer *buffer);

  /**
   * Skips ' ', '\t', '\r', and '\n'
   *
   * @param buffer
   */
  static void SkipWhitespace(ESB::Buffer *buffer);

  /**
   * Skips everything up to and including the first CRLF
   *
   * @param buffer The input buffer
   * @param bytesSkipped The number of bytes skipped not including the CRLF
   * @return ESB_AGAIN if a CRLF wasn't found in the buffer, ESB_SUCCESS
   * otherwise
   */
  static ESB::Error SkipLine(ESB::Buffer *buffer, ESB::UInt32 *bytesSkipped);

  /**
   * Skips ' ', '\t', and line continuations ([CRLF] 1*( SP | HT ))
   *
   * @return ESB_AGAIN if the parse cannot complete because there is not enough
   * data in the buffer, ESB_INPROGRESS if LWS was skipped (field value
   * continues), ESB_SUCCESS if a CRLF was skipped (end of field value), another
   * error code otherwise.
   */
  inline static ESB::Error SkipLWS(ESB::Buffer *buffer) {
    return SkipLWS(buffer, 3);  // Skip as much as 3 line continuations
  }

  static ESB::Error SkipLWS(ESB::Buffer *buffer, int depth);

  static bool IsSpace(unsigned char octet);

  static bool IsLWS(unsigned char octet);

  static bool IsMatch(ESB::Buffer *buffer, const unsigned char *literal);

  static bool EndsWith(const char *str, int strLength, const char *pattern, int patternLength);

  static ESB::Error ParseInteger(unsigned const char *str, ESB::UInt64 *integer);

  static ESB::Error FormatInteger(ESB::Buffer *buffer, ESB::UInt64 integer, int radix);

  inline static void Start(int *state, ESB::Buffer *outputBuffer, int initialState) {
    *state = initialState;

    outputBuffer->writeMark();
  }

  inline static ESB::Error Transition(int *state, ESB::Buffer *outputBuffer, int oldState, int newState) {
    *state &= ~oldState;
    *state |= newState;

    outputBuffer->writeMark();

    return ESB_SUCCESS;
  }

  inline static ESB::Error Rollback(ESB::Buffer *outputBuffer, ESB::Error result) {
    outputBuffer->writeReset();

    return result;
  }

/**
 * lowalpha = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" |
 * "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" |
 * "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"
 */
#define IS_LOW_ALPHA 0x01
/**
 * upalpha  = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" |
 * "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" |
 * "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
 */
#define IS_UP_ALPHA 0x02

/**
 * digit    = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
 */
#define IS_DIGIT 0x04

#define IS_ALPHA (IS_LOW_ALPHA | IS_UP_ALPHA)

#define IS_ALPHA_NUM (IS_ALPHA | IS_DIGIT)

/**
 * mark          = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"
 */
#define IS_MARK 0x08

#define IS_UNRESERVED (IS_ALPHA_NUM | IS_MARK)

/**
 * hex           = digit | "A" | "B" | "C" | "D" | "E" | "F" | "a" | "b" | "c" |
 * "d" | "e" | "f"
 */
#define IS_HEX (IS_DIGIT | 0x10)

/**
 * escaped       = "%" hex hex
 */
#define IS_ESCAPED (IS_HEX | 0x20)

/**
 * pchar         = unreserved | escaped | ":" | "@" | "&" | "=" | "+" | "$" |
 * ","
 */
#define IS_PCHAR (IS_UNRESERVED | IS_ESCAPED | 0x40)

/**
 * reserved      = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | ","
 */
#define IS_RESERVED 0x80
#define IS_URIC (IS_RESERVED | IS_UNRESERVED | IS_ESCAPED)

/**
 * TEXT           = <any OCTET except CTLs, but including LWS>
 * OCTET          = <any 8-bit sequence of data>
 * CTL            = <any US-ASCII control octet (octets 0 - 31) and DEL (127)>
 * LWS            = [CRLF] 1*( SP | HT )
 * <p/>
 * The TEXT rule is only used for descriptive field contents and values
 * that are not intended to be interpreted by the message parser. Words
 * of *TEXT MAY contain octets from octet sets other than ISO-
 * 8859-1 [22] only when encoded according to the rules of RFC 2047
 * [14].
 */
#define IS_TEXT 0x100

/**
 * separators     = "(" | ")" | "<" | ">" | "@" | "," | ";" | ":" | "\" | <"> |
 * "/" | "[" | "]" | "?" | "=" | "{" | "}" | SP | HT
 */
#define IS_SEPARATOR 0x200

/**
 * token          = 1*<any CHAR except CTLs or separators>
 * CHAR           = <any US-ASCII octet (octets 0 - 127)>
 * CTL            = <any US-ASCII control octet (octets 0 - 31) and DEL (127)>
 */
#define IS_TOKEN 0x400

  inline static bool IsPchar(unsigned char octet) { return _Bitmasks[octet] & IS_PCHAR; }

  inline static bool IsUnreserved(unsigned char octet) { return _Bitmasks[octet] & IS_UNRESERVED; }

  inline static bool IsEscaped(unsigned char octet) { return _Bitmasks[octet] & IS_ESCAPED; }

  inline static bool IsReserved(unsigned char octet) { return _Bitmasks[octet] & IS_RESERVED; }

  inline static bool IsUric(unsigned char octet) { return _Bitmasks[octet] & IS_URIC; }

  inline static bool IsAlpha(unsigned char octet) { return _Bitmasks[octet] & IS_ALPHA; }

  inline static bool IsAlphaNum(unsigned char octet) { return _Bitmasks[octet] & IS_ALPHA_NUM; }

  inline static bool IsLowAlpha(unsigned char octet) { return _Bitmasks[octet] & IS_LOW_ALPHA; }

  inline static bool IsUpAlpha(unsigned char octet) { return _Bitmasks[octet] & IS_UP_ALPHA; }

  inline static bool IsDigit(unsigned char octet) { return _Bitmasks[octet] & IS_DIGIT; }

  inline static bool IsMark(unsigned char octet) { return _Bitmasks[octet] & IS_MARK; }

  inline static bool IsHex(unsigned char octet) { return _Bitmasks[octet] & IS_HEX; }

  inline static bool IsText(unsigned char octet) { return _Bitmasks[octet] & IS_TEXT; }

  inline static bool IsSeparator(unsigned char octet) { return _Bitmasks[octet] & IS_SEPARATOR; }

  inline static bool IsToken(unsigned char octet) { return _Bitmasks[octet] & IS_TOKEN; }

 private:
  static ESB::UInt16 _Bitmasks[];
};

}  // namespace ES

#endif
