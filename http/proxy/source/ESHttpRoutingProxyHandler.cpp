#ifndef ES_HTTP_ROUTING_PROXY_HANDLER_H
#include <ESHttpRoutingProxyHandler.h>
#endif

#ifndef ES_HTTP_PROXY_CONTEXT_H
#include <ESHttpRoutingProxyContext.h>
#endif

namespace ES {

HttpRoutingProxyHandler::HttpRoutingProxyHandler(HttpRouter &router) : _router(router) {}

HttpRoutingProxyHandler::~HttpRoutingProxyHandler() {}

ESB::Error HttpRoutingProxyHandler::acceptConnection(HttpMultiplexer &multiplexer, ESB::SocketAddress *address) {
  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::beginTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream) {
  HttpRoutingProxyContext *context = new (serverStream.allocator()) HttpRoutingProxyContext();

  if (!context) {
    ESB_LOG_WARNING_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create proxy context", serverStream.logAddress());
    return ESB_OUT_OF_MEMORY;
  }

  assert(!serverStream.context());
  serverStream.setContext(context);
  context->setServerStream(&serverStream);

  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::receiveRequestHeaders(HttpMultiplexer &multiplexer,
                                                          HttpServerStream &serverStream) {
  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)serverStream.context();
  assert(context);
  if (!context) {
    return ESB_INVALID_STATE;
  }

  // TODO validate headers

  HttpClientTransaction *clientTransaction = multiplexer.createClientTransaction();

  if (!clientTransaction) {
    ESB_LOG_WARNING_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create client transaction", serverStream.logAddress());
    return serverStream.sendEmptyResponse(500, "Internal Server Error");
  }

  ESB::SocketAddress destination;
  ESB::Error error = _router.route(serverStream, *clientTransaction, destination);

  if (ESB_SUCCESS != error) {
    switch (error) {
      case ESB_CANNOT_FIND:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot route request", serverStream.logAddress());
        return serverStream.sendEmptyResponse(404, "Not Found");
      case ESB_NOT_OWNER:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot route request", serverStream.logAddress());
        return serverStream.sendEmptyResponse(403, "Forbidden");
      default:
        ESB_LOG_WARNING_ERRNO(error, "[%s] Cannot route request", serverStream.logAddress());
        return serverStream.sendEmptyResponse(500, "Internal Server Error");
    }
  }

  error = clientTransaction->request().copy(&serverStream.request(), clientTransaction->allocator());

  if (ESB_SUCCESS != error) {
    multiplexer.destroyClientTransaction(clientTransaction);
    ESB_LOG_WARNING_ERRNO(error, "[%s] Cannot populate client transaction", serverStream.logAddress());
    return serverStream.sendEmptyResponse(500, "Internal Server Error");
  }

  clientTransaction->setPeerAddress(destination);
  clientTransaction->setContext(context);

  error = multiplexer.executeClientTransaction(clientTransaction);

  if (ESB_SUCCESS != error) {
    multiplexer.destroyClientTransaction(clientTransaction);
    ESB_LOG_WARNING_ERRNO(error, "[%s] Cannot execute client transaction", serverStream.logAddress());
    return serverStream.sendEmptyResponse(500, "Internal Server Error");
  }

  // Pause the server transaction until we get the response from the client transaction

  return ESB_PAUSE;
}

ESB::Error HttpRoutingProxyHandler::receiveResponseHeaders(HttpMultiplexer &multiplexer,
                                                           HttpClientStream &clientStream) {
  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)clientStream.context();
  assert(context);
  if (!context) {
    return ESB_INVALID_STATE;
  }

  context->setClientStream(&clientStream);

  HttpServerStream *serverStream = context->serverStream();
  assert(serverStream);
  if (!serverStream) {
    return ESB_INVALID_STATE;
  }

  const HttpResponse &clientResponse = clientStream.response();

  ESB_LOG_DEBUG("[%s] received response, status=%d", clientStream.logAddress(), clientResponse.statusCode());

  switch (ESB::Error error = serverStream->sendResponse(clientResponse)) {
    case ESB_SUCCESS:
      break;
    case ESB_AGAIN:
      return ESB_AGAIN;
    default:
      ESB_LOG_WARNING_ERRNO(error, "[%s] cannot send server response", serverStream->logAddress());
      serverStream->abort(true);
      return error;
  }

  // TODO reuse connection for error responses

  if (300 <= clientResponse.statusCode()) {
    ESB_LOG_DEBUG("[%s] aborting server transaction due to error response", serverStream->logAddress());
    serverStream->abort(true);
    return ESB_CLOSED;
  }

  ESB::Error error = serverStream->resumeRecv(true);
  if (ESB_SUCCESS != error) {
    ESB_LOG_WARNING_ERRNO(error, "[%s] cannot resume server transaction", serverStream->logAddress());
    serverStream->abort(true);
    return error;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::consumeRequestBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                                       unsigned const char *body, ESB::UInt32 bytesOffered,
                                                       ESB::UInt32 *bytesConsumed) {
  if (!body || !bytesConsumed) {
    return ESB_NULL_POINTER;
  }

  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)serverStream.context();
  assert(context);
  if (!context) {
    return ESB_INVALID_STATE;
  }

  HttpClientStream *clientStream = context->clientStream();
  assert(clientStream);
  if (!clientStream) {
    return ESB_INVALID_STATE;
  }

  ESB::Error error = clientStream->sendRequestBody(body, bytesOffered, bytesConsumed);
  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot forward request body", clientStream->logAddress());
    return error;
  }

  ESB_LOG_DEBUG("[%s] Forwarded %u request body bytes", clientStream->logAddress(), *bytesConsumed);
  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                                      ESB::UInt32 *bytesAvailable) {
  if (!bytesAvailable) {
    return ESB_NULL_POINTER;
  }

  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)serverStream.context();
  assert(context);
  if (!context) {
    return ESB_INVALID_STATE;
  }

  HttpClientStream *clientStream = context->clientStream();
  assert(clientStream);
  if (!clientStream) {
    return ESB_INVALID_STATE;
  }

  ESB::UInt32 bufferOffset = 0U;
  ESB::Error error = clientStream->responseBodyAvailable(bytesAvailable, &bufferOffset);
  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot determine response body bytes available", clientStream->logAddress());
    return error;
  }

  context->setClientStreamResponseOffset(bufferOffset);

  ESB_LOG_DEBUG("[%s] %u response body bytes are available", clientStream->logAddress(), *bytesAvailable);
  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                                        unsigned char *body, ESB::UInt32 bytesRequested) {
  if (!body) {
    return ESB_NULL_POINTER;
  }

  if (0 >= bytesRequested) {
    return ESB_INVALID_ARGUMENT;
  }

  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)serverStream.context();
  assert(context);
  if (!context) {
    return ESB_INVALID_STATE;
  }

  HttpClientStream *clientStream = context->clientStream();
  assert(clientStream);
  if (!clientStream) {
    return ESB_INVALID_STATE;
  }

  ESB::Error error = clientStream->readResponseBody(body, bytesRequested, context->clientStreamResponseOffset());
  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot read %u response body bytes", clientStream->logAddress(), bytesRequested);
    return error;
  }

  ESB_LOG_DEBUG("[%s] read %u response body bytes", clientStream->logAddress(), bytesRequested);
  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::offerRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                                     ESB::UInt32 *bytesAvailable) {
  if (!bytesAvailable) {
    return ESB_NULL_POINTER;
  }

  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)clientStream.context();
  assert(context);
  if (!context) {
    return ESB_INVALID_STATE;
  }

  HttpServerStream *serverStream = context->serverStream();
  assert(serverStream);
  if (!serverStream) {
    return ESB_INVALID_STATE;
  }

  ESB::UInt32 bufferOffset = 0U;
  ESB::Error error = serverStream->requestBodyAvailable(bytesAvailable, &bufferOffset);
  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot determine request body bytes available", serverStream->logAddress());
    return error;
  }

  context->setServerStreamRequestOffset(bufferOffset);

  ESB_LOG_DEBUG("[%s] %u request body bytes are available", serverStream->logAddress(), *bytesAvailable);
  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::produceRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                                       unsigned char *body, ESB::UInt32 bytesRequested) {
  if (!body) {
    return ESB_NULL_POINTER;
  }

  if (0 >= bytesRequested) {
    return ESB_INVALID_ARGUMENT;
  }

  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)clientStream.context();
  assert(context);
  if (!context) {
    return ESB_INVALID_STATE;
  }

  HttpServerStream *serverStream = context->serverStream();
  assert(serverStream);
  if (!serverStream) {
    return ESB_INVALID_STATE;
  }

  ESB::Error error = serverStream->readRequestBody(body, bytesRequested, context->serverStreamRequestOffset());
  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot read %u request body bytes", serverStream->logAddress(), bytesRequested);
    return error;
  }

  ESB_LOG_DEBUG("[%s] read %u request body bytes", serverStream->logAddress(), bytesRequested);
  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::consumeResponseBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                                        const unsigned char *body, ESB::UInt32 bytesOffered,
                                                        ESB::UInt32 *bytesConsumed) {
  if (!body || !bytesConsumed) {
    return ESB_NULL_POINTER;
  }

  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)clientStream.context();
  assert(context);
  if (!context) {
    return ESB_INVALID_STATE;
  }

  HttpServerStream *serverStream = context->serverStream();
  assert(serverStream);
  if (!serverStream) {
    return ESB_INVALID_STATE;
  }

  ESB::Error error = serverStream->sendResponseBody(body, bytesOffered, bytesConsumed);
  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot forward response body", serverStream->logAddress());
    return error;
  }

  ESB_LOG_DEBUG("[%s] Forwarded %u response body bytes", serverStream->logAddress(), *bytesConsumed);
  return ESB_SUCCESS;
}

void HttpRoutingProxyHandler::endTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                             HttpClientHandler::State state) {
  switch (state) {
    case ES_HTTP_CLIENT_HANDLER_BEGIN:
      ESB_LOG_DEBUG("[%s] client transaction failed at begin state", clientStream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS:
      ESB_LOG_DEBUG("[%s] client transaction failed at response header parse state", clientStream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY:
      ESB_LOG_DEBUG("[%s] client transaction failed at request body parse state", clientStream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS:
      ESB_LOG_DEBUG("[%s] client transaction failed at response header send state", clientStream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY:
      ESB_LOG_DEBUG("[%s] client transaction failed at response header send state", clientStream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_END:
      ESB_LOG_DEBUG("[%s] client transaction successfully completed", clientStream.logAddress());
      break;
    default:
      ESB_LOG_WARNING("[%s] client transaction failed at unknown state", clientStream.logAddress());
  }

  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)clientStream.context();
  if (context) {
    // The server transaction has already received the endTransaction() and cleaned up
    return;
  }

  // The client transaction is the first to receive endTransaction(), so abort the server transaction

  HttpServerStream *serverStream = context->serverStream();
  assert(serverStream);
  if (!serverStream) {
    return;
  }

  context->~HttpRoutingProxyContext();
  serverStream->allocator().deallocate(context);
  serverStream->setContext(NULL);

  ESB::Error error = serverStream->abort(true);
  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] client transaction cannot abort associated server transaction [%s]",
                        clientStream.logAddress(), serverStream->logAddress());
  }
}

void HttpRoutingProxyHandler::endTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                             HttpServerHandler::State state) {
  switch (state) {
    case ES_HTTP_SERVER_HANDLER_BEGIN:
      ESB_LOG_DEBUG("[%s] server transaction failed at begin state", serverStream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS:
      ESB_LOG_DEBUG("[%s] server transaction failed at request header parse state", serverStream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY:
      ESB_LOG_DEBUG("[%s] server transaction failed at request body parse state", serverStream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS:
      ESB_LOG_DEBUG("[%s] server transaction failed at response header send state", serverStream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY:
      ESB_LOG_DEBUG("[%s] server transaction failed at response header send state", serverStream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_END:
      ESB_LOG_DEBUG("[%s] server transaction successfully completed", serverStream.logAddress());
      break;
    default:
      ESB_LOG_WARNING("[%s] server transaction failed at unknown state", serverStream.logAddress());
  }

  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)serverStream.context();
  if (context) {
    // The client transaction has already received the endTransaction() and cleaned up
    return;
  }

  // The server transaction is the first to receive endTransaction(), so abort the client transaction

  context->~HttpRoutingProxyContext();
  serverStream.allocator().deallocate(context);
  serverStream.setContext(NULL);

  HttpClientStream *clientStream = context->clientStream();
  assert(clientStream);
  if (!clientStream) {
    return;
  }

  ESB::Error error = clientStream->abort(true);
  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] server transaction cannot abort associated client transaction [%s]",
                        serverStream.logAddress(), clientStream->logAddress());
  }
}

ESB::Error HttpRoutingProxyHandler::endRequest(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) {
  return ESB_SUCCESS;
}

}  // namespace ES
