#ifndef ESB_ERROR_H
#include <ESBError.h>
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

namespace ESB {

#ifdef HAVE_ERRNO

Error LastError() { return ConvertError(errno); }

Error ConvertError(int error) {
  switch (error) {
    case 0:
      return ESB_SUCCESS;
    case EAGAIN:
#ifndef EAGAIN_AND_EWOULDBLOCK_IDENTICAL
    case EWOULDBLOCK:
#endif
      return ESB_AGAIN;
    case EINTR:
      return ESB_INTR;
    case EINPROGRESS:
      return ESB_INPROGRESS;
    default:
      return error;
  }
}

#elif defined HAVE_GETLASTERROR

Error GetLastError() { return ConvertError(GetLastError()); }

Error ConvertError(int error) {
  switch (error) {
    case ERROR_SUCCESS:
      return ESB_SUCCESS;
    case WSAEWOULDBLOCK:
      return ESB_AGAIN;
    case WSAEINTR:
      return ESB_INTR;
    case WSAEINPROGRESS:
      return ESB_INPROGRESS;
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
                                          "Overflow",
                                          "Cannot convert",
                                          "Illegal Encoding",
                                          "Unsupported Charset",
                                          "Index out of bounds",
                                          "Shutdown in progress",
                                          "Cleanup",
                                          "Paused",
                                          "Response must be sent",
                                          "Must be closed",
                                          "Not implemented",
                                          "Do not call again",
                                          "Unsupported transport",
                                          "TLS handshake failed",
                                          "TLS session error",
                                          "TLS general error",
                                          "Underflow",
                                          "Cannot parse",
                                          "Missing field",
                                          "Invalid field"};

#include <stdio.h>
void DescribeError(Error error, char *buffer, int size) {
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

    if (0 == FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, formatBuffer, 64, 0)) {
      return;
    }

#else
#error "strerror or equivalent is required"
#endif
  } else if (((int)(sizeof(ErrorDescriptions) / sizeof(char *))) > (-1 * error)) {
    int i = 0;

    for (int j = -1 * error; i < (size - 1) && ErrorDescriptions[j][i]; ++i) {
      buffer[i] = ErrorDescriptions[j][i];
    }

    buffer[i] = '\0';
  } else {
    return;
  }
}

}  // namespace ESB
