/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_ERROR_H
#define AWS_HTTP_ERROR_H

// todo
#define AWS_HTTP_BAD_CRLF -100
#define AWS_HTTP_BAD_REQUEST_URI_ASTERISK -101
#define AWS_HTTP_BAD_REQUEST_URI_ABS_PATH -102
#define AWS_HTTP_BAD_REQUEST_URI_QUERY -103
#define AWS_HTTP_BAD_REQUEST_URI_SCHEME -104
#define AWS_HTTP_BAD_REQUEST_URI_HOST -105
#define AWS_HTTP_BAD_REQUEST_URI_PORT -106
#define AWS_HTTP_BAD_REQUEST_METHOD -107
#define AWS_HTTP_BAD_REQUEST_VERSION -108
#define AWS_HTTP_BAD_REQUEST_FIELD_NAME -109
#define AWS_HTTP_BAD_REQUEST_FIELD_VALUE -110
#define AWS_HTTP_BAD_CONTENT_LENGTH -111
#define AWS_HTTP_BAD_INTEGER -112
#define AWS_HTTP_BAD_REASON_PHRASE -113
#define AWS_HTTP_BAD_STATUS_CODE -114
#define AWS_HTTP_BAD_REQUEST_URI_FRAGMENT -115
#define AWS_HTTP_MULTIPART_NOT_SUPPORTED -116
#define AWS_HTTP_PARSER_JAMMED -117
#define AWS_HTTP_FORMATTER_JAMMED -118

inline bool AWSHttpIsHttpError(int error)
{
    return error <= AWS_HTTP_BAD_CRLF && error >= AWS_HTTP_FORMATTER_JAMMED;
}

#endif
