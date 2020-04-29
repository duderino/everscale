#ifndef ES_HTTP_LOADGEN_REQUEST_BUILDER_H
#define ES_HTTP_LOADGEN_REQUEST_BUILDER_H

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

namespace ES {

extern ESB::Error HttpLoadgenRequestBuilder(const char *host, int port,
                                            const char *absPath,
                                            const char *method,
                                            const char *contentType,
                                            HttpClientTransaction *transaction);
}

#endif
