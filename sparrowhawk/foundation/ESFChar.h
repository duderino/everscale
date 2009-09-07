/** @file ESFChar.h
 *  @brief ISO-Latin-1 and Unicode (UTF-8, UTF-16, UTF-32) character types.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:08 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESF_CHAR_H
#define ESF_CHAR_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

/** @defgroup charset I18n Strings
 */

/** An enumeration of supported character sets.
 *
 *  @defgroup charset
 */
typedef enum {
    ESF_US_ASCII = 3, /**< ISO-646-IRV (US ASCII) */
    ESF_ISO_8859_1 = 4, /**< ISO/IEC 8859-1:1998 */
#ifdef NOT_SUPPORTED_YET
    ESF_ISO_8859_2 = 2, /**< ISO/IEC 8859-2:1999 */
    ESF_ISO_8859_3 = 3, /**< ISO/IEC 8859-3:1999 */
    ESF_ISO_8859_4 = 4, /**< ISO/IEC 8859-4:1998 */
    ESF_ISO_8859_5 = 5, /**< ISO/IEC 8859-5:1999 */
    ESF_ISO_8859_6 = 6, /**< ISO/IEC 8859-6:1999 */
    ESF_ISO_8859_7 = 7, /**< ISO 8859-7:1987 */
    ESF_ISO_8859_8 = 8, /**< ISO/IEC 8859-8:1999 */
    ESF_ISO_8859_9 = 9, /**< ISO/IEC 8859-9:1999 */
    ESF_ISO_8859_10 = 10, /**< ISO/IEC 8859-10:1998 */
    ESF_ISO_8859_13 = 13, /**< ISO/IEC 8859-13:1998 */
    ESF_ISO_8859_14 = 14, /**< ISO/IEC 8859-14:1998 */
    ESF_ISO_8859_15 = 15, /**< ISO/IEC 8859-15:1999 */
    ESF_ISO_8859_16 = 16, /**< ISO/IEC 8859-16:2001 */
#endif
    ESF_UTF8 = 106, /**< UTF-8 */
    ESF_UTF16 = 1015, /**< UTF-16.  Host Byte Order if no byte
     *   order mark (BOM), otherwise Big Endian
     *   if first char is 0xFEFF, Little Endian
     *   if first char is 0xFFFE.
     */
    ESF_UTF16BE = 1013, /**< UTF-16 (Big Endian/Network Order) */
    ESF_UTF16LE = 1014, /**< UTF-16 (Little Endian) */
    ESF_UTF32 = 1017, /**< UTF-32.  Host Byte Order if no byte
     *   order mark (BOM), otherwise Big Endian
     *   if first char is 0xFEFF, Little Endian
     *   if first char is 0xFFFE.
     */
    ESF_UTF32BE = 1018, /**< UTF-32 (Big Endian/Network Order) */
    ESF_UTF32LE = 1019
/**< UTF-32 (Little Endian) */
} ESFCharset;

/** UTF-8 Character.  1-4 of these will encode a Unicode Scalar Value.  Unicode
 *  values in the inclusive range 0x00 to 0x7F encoded in UTF-8 are
 *  compatible with US-ASCII.  That is, US-ASCII can be treated as UTF-8, but
 *  UTF-8 can only be treated as US-ASCII if all Unicode values fall within the
 *  0x00 to 0x7F inclusive range.
 *
 *  @ingroup charset
 */
typedef ESFUInt8 ESFUtf8Char;

/** UTF-16 Character.  1-2 of these will encode a Unicode Scalar Value.  Byte
 *  order may be Big Endian or Little Endian.
 *
 *  @ingroup charset
 */
typedef ESFUInt16 ESFUtf16Char;

/** UTF-32 Character.  1 of these will encode a Unicode Scalar Value.  Byte
 *  order may be Big Endian or Little Endian.
 *
 *  @ingroup charset
 */
typedef ESFUInt32 ESFUtf32Char;

/** Unicode Value.  This is a Unicode Scalar Value.  It is the unencoded form
 *  of a Unicode character and is the most natural way to access characters in
 *  ESFWideString objects.
 *
 *  @ingroup charset
 */
typedef ESFUInt32 ESFUnicodeChar;

#endif
