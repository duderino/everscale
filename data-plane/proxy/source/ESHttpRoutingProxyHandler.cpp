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

  ESB_LOG_DEBUG("[%s] begin server transaction", serverStream.logAddress());

  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::receiveRequestHeaders(HttpMultiplexer &multiplexer,
                                                          HttpServerStream &serverStream) {
  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)serverStream.context();
  assert(context);
  assert(context->serverStream());
  if (!context || !context->serverStream()) {
    return ESB_INVALID_STATE;
  }

  HttpClientTransaction *clientTransaction = multiplexer.createClientTransaction();

  if (!clientTransaction) {
    ESB_LOG_WARNING_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create client transaction", serverStream.logAddress());
    return serverStream.sendEmptyResponse(500, "Internal Server Error");
  }

  // TODO filter out unwanted headers from the server request using HttpMessage::HeaderCopyFilter filter
  ESB::Error error = clientTransaction->request().copy(&serverStream.request(), clientTransaction->allocator());
  switch (error) {
    case ESB_SUCCESS:
      break;
    case ESB_INVALID_FIELD:
      multiplexer.destroyClientTransaction(clientTransaction);
      ESB_LOG_WARNING_ERRNO(error, "[%s] Aborting client transaction due to bad server request",
                            serverStream.logAddress());
      return serverStream.sendEmptyResponse(400, "Bad Request");
    default:
      multiplexer.destroyClientTransaction(clientTransaction);
      ESB_LOG_WARNING_ERRNO(error, "[%s] Cannot populate client transaction", serverStream.logAddress());
      return serverStream.sendEmptyResponse(500, "Internal Server Error");
  }

  ESB::SocketAddress destination;
  error = _router.route(serverStream, *clientTransaction, destination);

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

  // Pause the server transaction until we get the response from the client transaction
  error = serverStream.pauseRecv(false);
  if (ESB_SUCCESS == error) {
    error = serverStream.pauseSend(true);
  }
  if (ESB_SUCCESS != error) {
    multiplexer.destroyClientTransaction(clientTransaction);
    ESB_LOG_WARNING_ERRNO(error, "[%s] cannot pause server stream", serverStream.logAddress());
    return error;
  }

  ESB_LOG_DEBUG("[%s] paused server stream", serverStream.logAddress());
  clientTransaction->setPeerAddress(destination);
  clientTransaction->setContext(context);

  error = multiplexer.executeClientTransaction(clientTransaction);

  if (ESB_SUCCESS != error) {
    multiplexer.destroyClientTransaction(clientTransaction);
    ESB_LOG_WARNING_ERRNO(error, "[%s] Cannot execute client transaction", serverStream.logAddress());
    return serverStream.sendEmptyResponse(500, "Internal Server Error");
  }

  // TODO optimization: if client connection is reused from pool, immediately send http request on it instead of waiting
  // for epoll to say it's writable

  return ESB_PAUSE;
}

ESB::Error HttpRoutingProxyHandler::beginTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) {
  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)clientStream.context();
  if (!context || !context->serverStream()) {
    // May happen if the server stream is aborted while the client stream is trying to connect
    return ESB_INVALID_STATE;
  }

  context->setClientStream(&clientStream);
  ESB_LOG_DEBUG("inbound [%s] has been paired with outbound [%s]", context->serverStream()->logAddress(),
                clientStream.logAddress());
  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::receiveResponseHeaders(HttpMultiplexer &multiplexer,
                                                           HttpClientStream &clientStream) {
  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)clientStream.context();
  assert(context);
  assert(context->serverStream());
  assert(context->clientStream());
  if (!context || !context->serverStream() || !context->clientStream()) {
    return ESB_INVALID_STATE;
  }

  context->setReceivedOutboundResponse(true);
  HttpServerStream &serverStream = *context->serverStream();

  ESB::Error error = serverStream.resumeRecv(true);
  if (ESB_SUCCESS != error) {
    ESB_LOG_WARNING_ERRNO(error, "[%s] cannot resume server stream", serverStream.logAddress());
    return error;
  }

  const HttpResponse &clientResponse = clientStream.response();
  ESB_LOG_DEBUG("[%s] received response, status=%d", clientStream.logAddress(), clientResponse.statusCode());
  ESB_LOG_DEBUG("[%s] server stream resumed", serverStream.logAddress());

  // TODO filter out unwanted response headers from the origin using HttpMessage::HeaderCopyFilter
  switch (error = serverStream.sendResponse(clientResponse)) {
    case ESB_SUCCESS:
      // response has been fully sent, so prepare server stream for reuse
      error = serverStream.resumeRecv(false);
      if (ESB_SUCCESS != error) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      error = serverStream.pauseSend(true);
      if (ESB_SUCCESS != error) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      break;
    case ESB_PAUSE:
      if (ESB_SUCCESS != (error = onClientRecvBlocked(serverStream, clientStream))) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      return ESB_PAUSE;
    case ESB_AGAIN:
      if (ESB_SUCCESS != (error = onServerSendBlocked(serverStream, clientStream))) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      return ESB_AGAIN;
    default:
      ESB_LOG_WARNING_ERRNO(error, "[%s] cannot send server response", serverStream.logAddress());
      return error;
  }

  // TODO reuse connection for error responses

  if (300 <= clientResponse.statusCode()) {
    ESB_LOG_DEBUG("[%s] aborting server transaction due to error response", serverStream.logAddress());
    return ESB_CLOSED;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::consumeRequestBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                                       unsigned const char *body, ESB::UInt64 bytesOffered,
                                                       ESB::UInt64 *bytesConsumed) {
  if (!body || !bytesConsumed) {
    return ESB_NULL_POINTER;
  }

  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)serverStream.context();
  if (!context || !context->clientStream()) {
    ESB_LOG_WARNING_ERRNO(ESB_INVALID_STATE, "[%s] consumed inbound request body before outbound response received",
                          serverStream.logAddress());
    assert(context);
    assert(context->clientStream());
    return ESB_INVALID_STATE;
  }

  HttpClientStream &clientStream = *context->clientStream();

  if (!context->receivedOutboundResponse()) {
    // Pause the server transaction until we get the response from the client transaction
    ESB_LOG_DEBUG("[%s] client send blocked on %lu request body bytes", clientStream.logAddress(), *bytesConsumed);
    ESB::Error error = onClientSendBlocked(serverStream, clientStream);
    if (ESB_SUCCESS != error) {
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
    return ESB_AGAIN;
  }

  *bytesConsumed = 0;
  ESB::Error error = clientStream.sendRequestBody(body, bytesOffered, bytesConsumed);

  if (0 < *bytesConsumed) {
    context->addRequestBodyBytesForwarded(*bytesConsumed);
    ESB_LOG_DEBUG("[%s] forwarding %lu/%lu request body bytes", clientStream.logAddress(), *bytesConsumed,
                  context->requestBodyBytesForwarded());
  }

  switch (error) {
    case ESB_SUCCESS:
      return ESB_SUCCESS;
    case ESB_PAUSE:
      ESB_LOG_DEBUG("[%s] server recv blocked on %lu request body bytes", clientStream.logAddress(), *bytesConsumed);
      if (ESB_SUCCESS != (error = onServerRecvBlocked(serverStream, clientStream))) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      return ESB_PAUSE;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] client send blocked on %lu request body bytes", clientStream.logAddress(), *bytesConsumed);
      if (ESB_SUCCESS != (error = onClientSendBlocked(serverStream, clientStream))) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      return ESB_AGAIN;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot forward request body", clientStream.logAddress());
      return error;
  }
}

ESB::Error HttpRoutingProxyHandler::offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                                      ESB::UInt64 *bytesAvailable) {
  if (!bytesAvailable) {
    return ESB_NULL_POINTER;
  }

  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)serverStream.context();
  assert(context);
  assert(context->clientStream());
  if (!context || !context->clientStream()) {
    return ESB_INVALID_STATE;
  }

  HttpClientStream &clientStream = *context->clientStream();

  switch (ESB::Error error = clientStream.responseBodyAvailable(bytesAvailable)) {
    case ESB_SUCCESS:
      ESB_LOG_DEBUG("[%s] %lu response body bytes are available", clientStream.logAddress(), *bytesAvailable);
      return ESB_SUCCESS;
    case ESB_PAUSE:
      if (ESB_SUCCESS != (error = onServerSendBlocked(serverStream, clientStream))) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      return ESB_PAUSE;
    case ESB_AGAIN:
      if (ESB_SUCCESS != (error = onClientRecvBlocked(serverStream, clientStream))) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      return ESB_AGAIN;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot determine response body bytes available", clientStream.logAddress());
      return error;
  }
}

ESB::Error HttpRoutingProxyHandler::produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                                        unsigned char *body, ESB::UInt64 bytesRequested) {
  if (!body) {
    return ESB_NULL_POINTER;
  }

  if (0 >= bytesRequested) {
    return ESB_INVALID_ARGUMENT;
  }

  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)serverStream.context();
  assert(context);
  assert(context->clientStream());
  if (!context || !context->clientStream()) {
    return ESB_INVALID_STATE;
  }

  HttpClientStream &clientStream = *context->clientStream();
  ESB::UInt64 bytesRead = 0U;
  ESB::Error error = clientStream.readResponseBody(body, bytesRequested, &bytesRead);

  switch (error) {
    case ESB_SUCCESS:
      break;
    case ESB_PAUSE:
      if (ESB_SUCCESS != (error = onServerSendBlocked(serverStream, clientStream))) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      break;
    case ESB_AGAIN:
      if (ESB_SUCCESS != (error = onClientRecvBlocked(serverStream, clientStream))) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      break;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot read %lu response body bytes", clientStream.logAddress(), bytesRequested);
      return error;
  }

  assert(bytesRequested == bytesRead);  // calling responseBodyAvailable before readResponseBody guarantees this
  context->addResponseBodyBytesForwarded(bytesRead);
  ESB_LOG_DEBUG("[%s] forwarding %lu/%lu response body bytes", serverStream.logAddress(), bytesRead,
                context->responseBodyBytesForwarded());
  return error;
}

ESB::Error HttpRoutingProxyHandler::offerRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                                     ESB::UInt64 *bytesAvailable) {
  if (!bytesAvailable) {
    return ESB_NULL_POINTER;
  }

  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)clientStream.context();
  assert(context);
  assert(context->serverStream());
  if (!context || !context->serverStream()) {
    return ESB_INVALID_STATE;
  }

  HttpServerStream &serverStream = *context->serverStream();

  switch (ESB::Error error = serverStream.requestBodyAvailable(bytesAvailable)) {
    case ESB_SUCCESS:
      ESB_LOG_DEBUG("[%s] %lu request body bytes are available", serverStream.logAddress(), *bytesAvailable);
      return ESB_SUCCESS;
    case ESB_PAUSE:
      if (ESB_SUCCESS != (error = onClientSendBlocked(serverStream, clientStream))) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      return ESB_PAUSE;
    case ESB_AGAIN:
      if (ESB_SUCCESS != (error = onServerRecvBlocked(serverStream, clientStream))) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      return ESB_AGAIN;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot determine request body bytes available", serverStream.logAddress());
      return error;
  }
}

ESB::Error HttpRoutingProxyHandler::produceRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                                       unsigned char *body, ESB::UInt64 bytesRequested) {
  if (!body) {
    return ESB_NULL_POINTER;
  }

  if (0 >= bytesRequested) {
    return ESB_INVALID_ARGUMENT;
  }

  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)clientStream.context();
  assert(context);
  assert(context->serverStream());
  if (!context || !context->serverStream()) {
    return ESB_INVALID_STATE;
  }

  HttpServerStream &serverStream = *context->serverStream();
  ESB::UInt64 bytesRead = 0U;
  ESB::Error error = serverStream.readRequestBody(body, bytesRequested, &bytesRead);

  switch (error) {
    case ESB_SUCCESS:
      break;
    case ESB_PAUSE:
      if (ESB_SUCCESS != (error = onClientSendBlocked(serverStream, clientStream))) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      break;
    case ESB_AGAIN:
      if (ESB_SUCCESS != (error = onServerRecvBlocked(serverStream, clientStream))) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      break;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot read %lu request body bytes", serverStream.logAddress(), bytesRequested);
      return error;
  }

  assert(bytesRequested == bytesRead);  // calling requestBodyAvailable before readRequestBody guarantees this
  context->addRequestBodyBytesForwarded(bytesRead);
  ESB_LOG_DEBUG("[%s] forwarding %lu/%lu request body bytes", clientStream.logAddress(), bytesRead,
                context->requestBodyBytesForwarded());
  return error;
}

ESB::Error HttpRoutingProxyHandler::consumeResponseBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                                        const unsigned char *body, ESB::UInt64 bytesOffered,
                                                        ESB::UInt64 *bytesConsumed) {
  if (!body || !bytesConsumed) {
    return ESB_NULL_POINTER;
  }

  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)clientStream.context();
  assert(context);
  assert(context->serverStream());
  if (!context || !context->serverStream()) {
    return ESB_INVALID_STATE;
  }

  HttpServerStream &serverStream = *context->serverStream();

  *bytesConsumed = 0;
  ESB::Error error = serverStream.sendResponseBody(body, bytesOffered, bytesConsumed);

  if (0 < *bytesConsumed) {
    context->addResponseBodyBytesForwarded(*bytesConsumed);
    ESB_LOG_DEBUG("[%s] forwarding %lu/%lu response body bytes", serverStream.logAddress(), *bytesConsumed,
                  context->responseBodyBytesForwarded());
  }

  switch (error) {
    case ESB_SUCCESS:
      if (bytesOffered == 0) {
        // prepare server stream for reuse
        serverStream.resumeRecv(false);
        serverStream.pauseSend(true);
      }
      return ESB_SUCCESS;
    case ESB_PAUSE:
      if (ESB_SUCCESS != (error = onClientRecvBlocked(serverStream, clientStream))) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      return ESB_PAUSE;
    case ESB_AGAIN:
      if (ESB_SUCCESS != (error = onServerSendBlocked(serverStream, clientStream))) {
        return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
      }
      return ESB_AGAIN;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot forward response body", serverStream.logAddress());
      return error;
  }
}

void HttpRoutingProxyHandler::endTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                             HttpClientHandler::State state) {
  switch (state) {
    case ES_HTTP_CLIENT_HANDLER_BEGIN:
      ESB_LOG_DEBUG("[%s] client transaction failed at begin state", clientStream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_CONNECT:
      ESB_LOG_DEBUG("[%s] client transaction failed at connect state", clientStream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS:
      ESB_LOG_DEBUG("[%s] client transaction failed at response header send state", clientStream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY:
      ESB_LOG_DEBUG("[%s] client transaction failed at response header send state", clientStream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS:
      ESB_LOG_DEBUG("[%s] client transaction failed at response header parse state", clientStream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY:
      ESB_LOG_DEBUG("[%s] client transaction failed at request body parse state", clientStream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_END:
      ESB_LOG_DEBUG("[%s] client transaction successfully completed", clientStream.logAddress());
      break;
    default:
      ESB_LOG_WARNING("[%s] client transaction failed at unknown state", clientStream.logAddress());
  }

  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)clientStream.context();
  if (!context) {
    return;
  }
  context->setClientStream(NULL);
  clientStream.setContext(NULL);

  if (ES_HTTP_CLIENT_HANDLER_END != state) {
    HttpServerStream *serverStream = context->serverStream();
    if (serverStream) {
#ifdef ESB_CI_BUILD
      ESB_LOG_WARNING("[%s] aborting associated server stream [%s]", clientStream.logAddress(),
                      serverStream->logAddress());
#else
      ESB_LOG_DEBUG("[%s] aborting associated server stream [%s]", clientStream.logAddress(),
                    serverStream->logAddress());
#endif
      ESB::Error error = serverStream->abort();
      if (ESB_SUCCESS != error) {
        ESB_LOG_WARNING_ERRNO(error, "[%s] cannot abort associated server stream", clientStream.logAddress());
      }
      context->setServerStream(NULL);
    }
  }

  // destroy the context if the server stream no longer references it.

  HttpServerStream *serverStream = context->serverStream();
  if (!serverStream || !serverStream->context()) {
    context->~HttpRoutingProxyContext();
    clientStream.allocator().deallocate(context);
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
  if (!context) {
    return;
  }
  context->setServerStream(NULL);
  serverStream.setContext(NULL);

  if (ES_HTTP_SERVER_HANDLER_END != state) {
    HttpClientStream *clientStream = context->clientStream();
    if (clientStream) {
#ifdef ESB_CI_BUILD
      ESB_LOG_WARNING("[%s] aborting associated client stream [%s]", serverStream.logAddress(),
                      clientStream->logAddress());
#else
      ESB_LOG_DEBUG("[%s] aborting associated client stream [%s]", serverStream.logAddress(),
                    clientStream->logAddress());
#endif
      ESB::Error error = clientStream->abort();
      if (ESB_SUCCESS != error) {
        ESB_LOG_WARNING_ERRNO(error, "[%s] cannot abort associated client stream", serverStream.logAddress());
      }
      context->setClientStream(NULL);
    }
  }

  // destroy the context if the client stream no longer references it.

  HttpClientStream *clientStream = context->clientStream();
  if (!clientStream || !clientStream->context()) {
    context->~HttpRoutingProxyContext();
    serverStream.allocator().deallocate(context);
  }
}

ESB::Error HttpRoutingProxyHandler::endRequest(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) {
  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::onClientRecvBlocked(HttpServerStream &serverStream,
                                                        HttpClientStream &clientStream) {
  //     - when client recv blocks:  resume client recv, pause server send
  ESB::Error error;
  if (ESB_SUCCESS != (error = clientStream.resumeRecv(true))) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot resume client stream receive", clientStream.logAddress());
    return error;
  }
  if (ESB_SUCCESS != (error = serverStream.pauseSend(true))) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot pause server stream send", serverStream.logAddress());
    return error;
  }
  ESB_LOG_DEBUG("[%s] resumed client stream receive", clientStream.logAddress());
  ESB_LOG_DEBUG("[%s] paused server stream send", serverStream.logAddress());
  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::onServerRecvBlocked(HttpServerStream &serverStream,
                                                        HttpClientStream &clientStream) {
  //     - when server recv blocks:  resume server recv, pause client send
  ESB::Error error;
  if (ESB_SUCCESS != (error = serverStream.resumeRecv(true))) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot resume server stream receive", serverStream.logAddress());
    return error;
  }
  if (ESB_SUCCESS != (error = clientStream.pauseSend(true))) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot pause client stream send", clientStream.logAddress());
    return error;
  }
  ESB_LOG_DEBUG("[%s] resumed server stream receive", serverStream.logAddress());
  ESB_LOG_DEBUG("[%s] paused client stream send", clientStream.logAddress());
  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::onClientSendBlocked(HttpServerStream &serverStream,
                                                        HttpClientStream &clientStream) {
  //     - when client send blocks:  resume client send, pause server recv
  ESB::Error error;
  if (ESB_SUCCESS != (error = clientStream.resumeSend(true))) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot resume client stream send", clientStream.logAddress());
    return error;
  }
  if (ESB_SUCCESS != (error = serverStream.pauseRecv(true))) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot pause server stream receive", serverStream.logAddress());
    return error;
  }
  ESB_LOG_DEBUG("[%s] resumed client stream send", clientStream.logAddress());
  ESB_LOG_DEBUG("[%s] paused server stream receive", serverStream.logAddress());
  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::onServerSendBlocked(HttpServerStream &serverStream,
                                                        HttpClientStream &clientStream) {
  //     - when server send blocks:  resume server send, pause client recv
  ESB::Error error;
  if (ESB_SUCCESS != (error = serverStream.resumeSend(true))) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot resume server stream send", serverStream.logAddress());
    return error;
  }
  if (ESB_SUCCESS != (error = clientStream.pauseRecv(true))) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot pause client stream receive", clientStream.logAddress());
    return error;
  }
  ESB_LOG_DEBUG("[%s] resumed server stream send", serverStream.logAddress());
  ESB_LOG_DEBUG("[%s] paused client stream receive", clientStream.logAddress());
  return ESB_SUCCESS;
}

}  // namespace ES
