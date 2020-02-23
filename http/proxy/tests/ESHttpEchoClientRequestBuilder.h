#ifndef ES_HTTP_ECHO_CLIENT_REQUEST_BUILDER_H
#define ES_HTTP_ECHO_CLIENT_REQUEST_BUILDER_H

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

namespace ES {

extern ESB::Error HttpEchoClientRequestBuilder(
    const char *host, int port, const char *absPath, const char *method,
    const char *contentType, HttpClientTransaction *transaction);

}

#endif
