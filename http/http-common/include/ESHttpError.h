#ifndef ES_HTTP_ERROR_H
#define ES_HTTP_ERROR_H

namespace ES {

#define ES_HTTP_BAD_CRLF -100
#define ES_HTTP_BAD_REQUEST_URI_ASTERISK -101
#define ES_HTTP_BAD_REQUEST_URI_ABS_PATH -102
#define ES_HTTP_BAD_REQUEST_URI_QUERY -103
#define ES_HTTP_BAD_REQUEST_URI_SCHEME -104
#define ES_HTTP_BAD_REQUEST_URI_HOST -105
#define ES_HTTP_BAD_REQUEST_URI_PORT -106
#define ES_HTTP_BAD_REQUEST_METHOD -107
#define ES_HTTP_BAD_REQUEST_VERSION -108
#define ES_HTTP_BAD_REQUEST_FIELD_NAME -109
#define ES_HTTP_BAD_REQUEST_FIELD_VALUE -110
#define ES_HTTP_BAD_CONTENT_LENGTH -111
#define ES_HTTP_BAD_INTEGER -112
#define ES_HTTP_BAD_REASON_PHRASE -113
#define ES_HTTP_BAD_STATUS_CODE -114
#define ES_HTTP_BAD_REQUEST_URI_FRAGMENT -115
#define ES_HTTP_MULTIPART_NOT_SUPPORTED -116
#define ES_HTTP_PARSER_JAMMED -117
#define ES_HTTP_FORMATTER_JAMMED -118

inline bool IsHttpError(int error) {
  return error <= ES_HTTP_BAD_CRLF && error >= ES_HTTP_FORMATTER_JAMMED;
}

}  // namespace ES

#endif
