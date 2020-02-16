/** @file ESFError.cpp
 *  @brief An abstraction over system error codes and error reporting
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_ERROR_H
#include <ESFError.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#ifdef HAVE_TCHAR_H
#include <tchar.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_ERRNO

ESFError ESFGetLastError() { return ESFConvertError(errno); }

ESFError ESFConvertError(int error) {
  switch (error) {
    case 0:
      return ESF_SUCCESS;
    case EAGAIN:
#ifndef EAGAIN_AND_EWOULDBLOCK_IDENTICAL
    case EWOULDBLOCK:
#endif
      return ESF_AGAIN;
    case EINTR:
      return ESF_INTR;
    case EINPROGRESS:
      return ESF_INPROGRESS;
    default:
      return error;
  }
}

#elif defined HAVE_GETLASTERROR

ESFError ESFGetLastError() { return ESFConvertError(GetLastError()); }

ESFError ESFConvertError(int error) {
  switch (error) {
    case ERROR_SUCCESS:
      return ESF_SUCCESS;
    case WSAEWOULDBLOCK:
      return ESF_AGAIN;
    case WSAEINTR:
      return ESF_INTR;
    case WSAEINPROGRESS:
      return ESF_INPROGRESS;
    default:
      return error;
  }
}

#else
#error "Errno or equivalent is required"
#endif

static const char *ErrorDescriptions[] = {"Success",
                                          "Other error",
                                          "Operation not supported",
                                          "Null pointer",
                                          "Uniqueness violation",
                                          "Invalid argument",
                                          "Out of memory",
                                          "Not initialized",
                                          "Operation would block",
                                          "Operation interrupted",
                                          "Operation in progress",
                                          "Operation timed out",
                                          "Argument too short",
                                          "Result truncated",
                                          "Invalid state",
                                          "Not owner",
                                          "Resources in use",
                                          "Cannot find item",
                                          "Invalid iterator",
                                          "Hard limit reached",
                                          "Cannot convert",
                                          "Illegal Encoding",
                                          "Unsupported Charset",
                                          "Index out of bounds",
                                          "Shutdown in progress",
                                          "Partial success"};

#include <stdio.h>
void ESFDescribeError(ESFError error, char *buffer, int size) {
  if (!buffer || 1 > size) {
    return;
  }

  buffer[0] = '\0';

  if (0 < error) {
#ifdef HAVE_STRERROR_R

    char *description = strerror_r(error, (char *)buffer, size);

    if (description) {
      strncpy(buffer, description, size - 1);
      buffer[size - 1] = 0;
    } else {
      buffer[0] = 0;
    }

#elif defined HAVE_STRERROR

    char *description = strerror(error);

    if (!description) {
      return;
    }

    int i = 0;

    for (; i < (size - 1) && description[i]; ++i) {
      buffer[i] = description[i];
    }

    buffer[i] = '\0';

#elif defined HAVE_FORMAT_MESSAGE && defined HAVE_TCHAR

#error "must convert this to utf-8"

    TCHAR formatBuffer[64];

    if (0 == FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0,
                           formatBuffer, 64, 0)) {
      return;
    }

#else
#error "strerror or equivalent is required"
#endif
  } else if (((int)(sizeof(ErrorDescriptions) / sizeof(char *))) >
             (-1 * error)) {
    int i = 0;

    for (int j = -1 * error; i < (size - 1) && ErrorDescriptions[j][i]; ++i) {
      buffer[i] = ErrorDescriptions[j][i];
    }

    buffer[i] = '\0';
  } else {
    return;
  }
}
