#ifndef ESB_CHAR_H
#define ESB_CHAR_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

namespace ESB {

/** @defgroup charset I18n Strings
 */

/** An enumeration of supported character sets.
 *
 *  @defgroup charset
 */
typedef enum {
  ESB_US_ASCII = 3,   /**< ISO-646-IRV (US ASCII) */
  ESB_ISO_8859_1 = 4, /**< ISO/IEC 8859-1:1998 */
#ifdef NOT_SUPPORTED_YET
  ESB_ISO_8859_2 = 2,   /**< ISO/IEC 8859-2:1999 */
  ESB_ISO_8859_3 = 3,   /**< ISO/IEC 8859-3:1999 */
  ESB_ISO_8859_4 = 4,   /**< ISO/IEC 8859-4:1998 */
  ESB_ISO_8859_5 = 5,   /**< ISO/IEC 8859-5:1999 */
  ESB_ISO_8859_6 = 6,   /**< ISO/IEC 8859-6:1999 */
  ESB_ISO_8859_7 = 7,   /**< ISO 8859-7:1987 */
  ESB_ISO_8859_8 = 8,   /**< ISO/IEC 8859-8:1999 */
  ESB_ISO_8859_9 = 9,   /**< ISO/IEC 8859-9:1999 */
  ESB_ISO_8859_10 = 10, /**< ISO/IEC 8859-10:1998 */
  ESB_ISO_8859_13 = 13, /**< ISO/IEC 8859-13:1998 */
  ESB_ISO_8859_14 = 14, /**< ISO/IEC 8859-14:1998 */
  ESB_ISO_8859_15 = 15, /**< ISO/IEC 8859-15:1999 */
  ESB_ISO_8859_16 = 16, /**< ISO/IEC 8859-16:2001 */
#endif
  ESB_UTF8 = 106,     /**< UTF-8 */
  ESB_UTF16 = 1015,   /**< UTF-16.  Host Byte Order if no byte
                       *   order mark (BOM), otherwise Big Endian
                       *   if first char is 0xFEFF, Little Endian
                       *   if first char is 0xFFFE.
                       */
  ESB_UTF16BE = 1013, /**< UTF-16 (Big Endian/Network Order) */
  ESB_UTF16LE = 1014, /**< UTF-16 (Little Endian) */
  ESB_UTF32 = 1017,   /**< UTF-32.  Host Byte Order if no byte
                       *   order mark (BOM), otherwise Big Endian
                       *   if first char is 0xFEFF, Little Endian
                       *   if first char is 0xFFFE.
                       */
  ESB_UTF32BE = 1018, /**< UTF-32 (Big Endian/Network Order) */
  ESB_UTF32LE = 1019
  /**< UTF-32 (Little Endian) */
} Charset;

/** UTF-8 Character.  1-4 of these will encode a Unicode Scalar Value.  Unicode
 *  values in the inclusive range 0x00 to 0x7F encoded in UTF-8 are
 *  compatible with US-ASCII.  That is, US-ASCII can be treated as UTF-8, but
 *  UTF-8 can only be treated as US-ASCII if all Unicode values fall within the
 *  0x00 to 0x7F inclusive range.
 *
 *  @ingroup charset
 */
typedef UInt8 Utf8Char;

/** UTF-16 Character.  1-2 of these will encode a Unicode Scalar Value.  Byte
 *  order may be Big Endian or Little Endian.
 *
 *  @ingroup charset
 */
typedef UInt16 Utf16Char;

/** UTF-32 Character.  1 of these will encode a Unicode Scalar Value.  Byte
 *  order may be Big Endian or Little Endian.
 *
 *  @ingroup charset
 */
typedef UInt32 Utf32Char;

/** Unicode Value.  This is a Unicode Scalar Value.  It is the unencoded form
 *  of a Unicode character and is the most natural way to access characters in
 *  ESBWideString objects.
 *
 *  @ingroup charset
 */
typedef UInt32 UnicodeChar;

}  // namespace ESB

#endif
