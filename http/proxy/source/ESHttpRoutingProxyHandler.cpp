#ifndef ES_HTTP_ROUTING_PROXY_HANDLER_H
#include <ESHttpRoutingProxyHandler.h>
#endif

#ifndef ES_HTTP_PROXY_CONTEXT_H
#include <ESHttpRoutingProxyContext.h>
#endif

namespace ES {

HttpRoutingProxyHandler::HttpRoutingProxyHandler(HttpRouter &router)
    : _router(router) {}

HttpRoutingProxyHandler::~HttpRoutingProxyHandler() {}

ESB::Error HttpRoutingProxyHandler::acceptConnection(
    HttpMultiplexer &multiplexer, ESB::SocketAddress *address) {
  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::beginServerTransaction(
    HttpMultiplexer &multiplexer, HttpServerStream &stream) {
  HttpRoutingProxyContext *context =
      new (stream.allocator()) HttpRoutingProxyContext();

  if (!context) {
    ESB_LOG_WARNING_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create proxy context",
                          stream.logAddress());
    return ESB_OUT_OF_MEMORY;
  }

  assert(!stream.context());
  stream.setContext(context);
  context->setServerStream(&stream);

  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::receiveRequestHeaders(
    HttpMultiplexer &multiplexer, HttpServerStream &stream) {
  HttpRoutingProxyContext *context =
      (HttpRoutingProxyContext *)stream.context();
  assert(context);

  // TODO validate headers

  HttpClientTransaction *transaction = multiplexer.createClientTransaction();

  if (!transaction) {
    ESB_LOG_WARNING_ERRNO(ESB_OUT_OF_MEMORY,
                          "[%s] Cannot create client transaction",
                          stream.logAddress());
    return sendResponse(stream, 500, "Internal Server Error");
  }

  ESB::SocketAddress destination;
  ESB::Error error = _router.route(stream, *transaction, destination);

  if (ESB_SUCCESS != error) {
    switch (error) {
      case ESB_CANNOT_FIND:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot route request",
                            stream.logAddress());
        return sendResponse(stream, 404, "Not Found");
      case ESB_NOT_OWNER:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot route request",
                            stream.logAddress());
        return sendResponse(stream, 403, "Forbidden");
      default:
        ESB_LOG_WARNING_ERRNO(error, "[%s] Cannot route request",
                              stream.logAddress());
        return sendResponse(stream, 500, "Internal Server Error");
    }
  }

  transaction->setContext(context);
  error =
      transaction->request().copy(&stream.request(), transaction->allocator());

  if (ESB_SUCCESS != error) {
    multiplexer.destroyClientTransaction(transaction);
    ESB_LOG_WARNING_ERRNO(error, "[%s] Cannot populate client transaction",
                          stream.logAddress());
    return sendResponse(stream, 500, "Internal Server Error");
  }

  error = multiplexer.executeClientTransaction(transaction);

  if (ESB_SUCCESS != error) {
    multiplexer.destroyClientTransaction(transaction);
    ESB_LOG_WARNING_ERRNO(error, "[%s] Cannot execute client transaction",
                          stream.logAddress());
    return sendResponse(stream, 500, "Internal Server Error");
  }

  context->setState(HttpRoutingProxyContext::State::CLIENT_RESPONSE_WAIT);

  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::requestChunkCapacity(
    HttpMultiplexer &multiplexer, HttpServerStream &stream,
    ESB::UInt32 *maxChunkSize) {
  HttpRoutingProxyContext *context =
      (HttpRoutingProxyContext *)stream.context();
  assert(context);

  switch (context->state()) {
    case HttpRoutingProxyContext::State::CLIENT_RESPONSE_WAIT:
      // Wait until we the upstream response before accepting body data.  This
      // will pause the server socket
      return ESB_NOT_IMPLEMENTED;
    case HttpRoutingProxyContext::State::STREAMING:
      assert(context->clientStream());
      assert(0 ==
             "TODO modify HttpStream to return remaining space in the ssend "
             "buffer");
    default:
      assert(0 == "offerRequestChunk in invalid state");
      ESB_LOG_ERROR("[%s] transaction in invalid state", stream.logAddress());
      return ESB_NOT_IMPLEMENTED;
  }
}

ESB::Error HttpRoutingProxyHandler::receiveRequestChunk(
    HttpMultiplexer &multiplexer, HttpServerStream &stream,
    unsigned const char *chunk, ESB::UInt32 chunkSize) {
  assert(chunk);
  HttpRoutingProxyContext *context =
      (HttpRoutingProxyContext *)stream.context();
  assert(context);

  assert(0 == "TODO modify HttpStream to expose subset of formatRequestBody");

  HttpResponse &response = stream.response();

  if (0U == chunkSize) {
    response.setStatusCode(200);
    response.setReasonPhrase("OK");
    return ESB_SEND_RESPONSE;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::offerResponseChunk(
    HttpMultiplexer &multiplexer, HttpServerStream &stream,
    ESB::UInt32 *maxChunkSize) {
  HttpRoutingProxyContext *context =
      (HttpRoutingProxyContext *)stream.context();
  assert(context);
  return ESB_NOT_IMPLEMENTED;
}

ESB::Error HttpRoutingProxyHandler::takeResponseChunk(
    HttpMultiplexer &multiplexer, HttpServerStream &stream,
    unsigned char *chunk, ESB::UInt32 chunkSize) {
  assert(chunk);
  assert(0 < chunkSize);
  HttpRoutingProxyContext *context =
      (HttpRoutingProxyContext *)stream.context();
  assert(context);
  return ESB_NOT_IMPLEMENTED;
}

void HttpRoutingProxyHandler::endServerTransaction(
    HttpMultiplexer &multiplexer, HttpServerStream &stream,
    HttpServerHandler::State state) {
  HttpRoutingProxyContext *context =
      (HttpRoutingProxyContext *)stream.context();
  assert(context);
  ESB::Allocator &allocator = stream.allocator();

  context->~HttpRoutingProxyContext();
  allocator.deallocate(context);
  stream.setContext(NULL);

  switch (state) {
    case ES_HTTP_SERVER_HANDLER_BEGIN:
      ESB_LOG_INFO("Transaction failed at begin state");
      break;
    case ES_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS:
      ESB_LOG_INFO("Transaction failed at request header parse state");
      break;
    case ES_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY:
      ESB_LOG_INFO("Transaction failed at request body parse state");
      break;
    case ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS:
      ESB_LOG_INFO("Transaction failed at response header send state");
      break;
    case ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY:
      ESB_LOG_INFO("Transaction failed at response header send state");
      break;
    case ES_HTTP_SERVER_HANDLER_END:
      break;
    default:
      ESB_LOG_WARNING("Transaction failed at unknown state");
  }
}

ESB::Error HttpRoutingProxyHandler::offerRequestChunk(
    HttpMultiplexer &multiplexer, HttpClientStream &stream,
    ESB::UInt32 *maxChunkSize) {
  return ESB_NOT_IMPLEMENTED;
}

ESB::Error HttpRoutingProxyHandler::takeResponseChunk(
    HttpMultiplexer &multiplexer, HttpClientStream &stream,
    unsigned char *chunk, ESB::UInt32 chunkSize) {
  return ESB_NOT_IMPLEMENTED;
}

ESB::Error HttpRoutingProxyHandler::receiveResponseHeaders(
    HttpMultiplexer &multiplexer, HttpClientStream &stream) {
  // TODO unpause or close server stream.  if not paused, just write?
  return ESB_NOT_IMPLEMENTED;
}

ESB::Error HttpRoutingProxyHandler::responseChunkCapacity(
    HttpMultiplexer &multiplexer, HttpClientStream &stream,
    ESB::UInt32 *maxChunkSize) {
  return ESB_NOT_IMPLEMENTED;
}

ESB::Error HttpRoutingProxyHandler::receiveResponseChunk(
    HttpMultiplexer &multiplexer, HttpClientStream &stream,
    unsigned const char *chunk, ESB::UInt32 chunkSize) {
  return ESB_NOT_IMPLEMENTED;
}

void HttpRoutingProxyHandler::endClientTransaction(
    HttpMultiplexer &multiplexer, HttpClientStream &stream,
    HttpClientHandler::State state) {}

ESB::Error HttpRoutingProxyHandler::sendResponse(HttpServerStream &stream,
                                                 int statusCode,
                                                 const char *reasonPhrase) {
  stream.response().setStatusCode(500);
  stream.response().setReasonPhrase("Internal Server Error");
  return ESB_SEND_RESPONSE;
}

}  // namespace ES
