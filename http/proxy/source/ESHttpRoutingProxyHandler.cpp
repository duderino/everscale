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

ESB::Error HttpRoutingProxyHandler::beginTransaction(HttpMultiplexer &multiplexer, HttpServerStream &stream) {
  HttpRoutingProxyContext *context = new (stream.allocator()) HttpRoutingProxyContext();

  if (!context) {
    ESB_LOG_WARNING_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create proxy context", stream.logAddress());
    return ESB_OUT_OF_MEMORY;
  }

  assert(!stream.context());
  stream.setContext(context);
  context->setServerStream(&stream);

  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::receiveRequestHeaders(HttpMultiplexer &multiplexer,
                                                          HttpServerStream &serverStream) {
  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)serverStream.context();
  assert(context);

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

  context->setState(HttpRoutingProxyContext::State::CLIENT_RESPONSE_WAIT);

  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::consumeRequestBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                                       unsigned const char *chunk, ESB::UInt32 chunkSize,
                                                       ESB::UInt32 *bytesConsumed) {
  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)stream.context();
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
      assert(0 == "offerRequestBody in invalid state");
      ESB_LOG_ERROR("[%s] transaction in invalid state", stream.logAddress());
      return ESB_NOT_IMPLEMENTED;
  }

  assert(0 == "TODO modify HttpStream to expose subset of formatRequestBody");

  HttpResponse &response = stream.response();

  if (0U == chunkSize) {
    response.setStatusCode(200);
    response.setReasonPhrase("OK");
    return ESB_SEND_RESPONSE;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpRoutingProxyHandler::offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                                      ESB::UInt32 *bytesAvailable) {
  return ESB_NOT_IMPLEMENTED;
}

ESB::Error HttpRoutingProxyHandler::produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                                        unsigned char *chunk, ESB::UInt32 bytesRequested) {
  assert(chunk);
  assert(0 < bytesRequested);
  assert(stream.context());
  return ESB_NOT_IMPLEMENTED;
}

void HttpRoutingProxyHandler::endTransaction(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                             HttpServerHandler::State state) {
  HttpRoutingProxyContext *context = (HttpRoutingProxyContext *)stream.context();
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

ESB::Error HttpRoutingProxyHandler::receiveResponseHeaders(HttpMultiplexer &multiplexer, HttpClientStream &stream) {
  // TODO unpause or close server stream.  if not paused, just write?
  return ESB_NOT_IMPLEMENTED;
}

ESB::Error HttpRoutingProxyHandler::offerRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &stream,
                                                     ESB::UInt32 *bytesAvailable) {
  return ESB_NOT_IMPLEMENTED;
}

ESB::Error HttpRoutingProxyHandler::produceRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &stream,
                                                       unsigned char *chunk, ESB::UInt32 bytesRequested) {
  return ESB_NOT_IMPLEMENTED;
}

ESB::Error HttpRoutingProxyHandler::consumeResponseBody(HttpMultiplexer &multiplexer, HttpClientStream &stream,
                                                        const unsigned char *chunk, ESB::UInt32 chunkSize,
                                                        ESB::UInt32 *bytesConsumed) {
  return ESB_NOT_IMPLEMENTED;
}

void HttpRoutingProxyHandler::endTransaction(HttpMultiplexer &multiplexer, HttpClientStream &stream,
                                             HttpClientHandler::State state) {}

ESB::Error HttpRoutingProxyHandler::sendResponse(HttpServerStream &stream, int statusCode, const char *reasonPhrase) {
  stream.response().setStatusCode(500);
  stream.response().setReasonPhrase("Internal Server Error");
  return ESB_SEND_RESPONSE;
}

}  // namespace ES
