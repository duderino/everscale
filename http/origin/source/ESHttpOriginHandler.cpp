#ifndef ES_HTTP_ORIGIN_HANDLER_H
#include <ESHttpOriginHandler.h>
#endif

#ifndef ES_HTTP_ORIGIN_CONTEXT_H
#include <ESHttpOriginContext.h>
#endif

namespace ES {

HttpOriginHandler::HttpOriginHandler(HttpTestParams &params) : _params(params) {}

HttpOriginHandler::~HttpOriginHandler() {}

ESB::Error HttpOriginHandler::acceptConnection(HttpMultiplexer &stack, ESB::SocketAddress *address) {
  return ESB_SUCCESS;
}

ESB::Error HttpOriginHandler::beginTransaction(HttpMultiplexer &stack, HttpServerStream &stream) {
  ESB::Allocator &allocator = stream.allocator();
  HttpOriginContext *context = new (allocator) HttpOriginContext();

  if (!context) {
    ESB_LOG_WARNING_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot allocate new transaction", stream.logAddress());
    return ESB_OUT_OF_MEMORY;
  }

  assert(!stream.context());
  stream.setContext(context);
  return ESB_SUCCESS;
}

ESB::Error HttpOriginHandler::receiveRequestHeaders(HttpMultiplexer &stack, HttpServerStream &stream) {
  return ESB_SUCCESS;
}

ESB::Error HttpOriginHandler::consumeRequestBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                                 unsigned const char *chunk, ESB::UInt32 chunkSize,
                                                 ESB::UInt32 *bytesConsumed) {
  assert(chunk);
  assert(bytesConsumed);
  HttpOriginContext *context = (HttpOriginContext *)stream.context();
  assert(context);

  *bytesConsumed = chunkSize;
  context->setBytesReceived(context->bytesReceived() + chunkSize);

  HttpResponse &response = stream.response();

  if (0 < chunkSize) {
    return ESB_SUCCESS;
  }

  response.setStatusCode(200);
  response.setReasonPhrase("OK");

  ESB::Error error = response.addHeader("Transfer-Encoding", "chunked", stream.allocator());
  if (ESB_SUCCESS != error) {
    return error;
  }

  error = response.addHeader("Content-Type", "octet-stream", stream.allocator());
  if (ESB_SUCCESS != error) {
    return error;
  }

  return ESB_SEND_RESPONSE;
}

ESB::Error HttpOriginHandler::offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                                ESB::UInt32 *bytesAvailable) {
  HttpOriginContext *context = (HttpOriginContext *)stream.context();
  assert(context);
  *bytesAvailable = _params.responseSize() - context->bytesSent();
  return ESB_SUCCESS;
}

ESB::Error HttpOriginHandler::produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                                  unsigned char *chunk, ESB::UInt32 bytesRequested) {
  assert(chunk);
  assert(0 < bytesRequested);
  HttpOriginContext *context = (HttpOriginContext *)stream.context();
  assert(context);
  assert(bytesRequested <= _params.responseSize() - context->bytesSent());

  ESB::UInt32 totalBytesRemaining = _params.responseSize() - context->bytesSent();
  ESB::UInt32 bytesToSend = bytesRequested > totalBytesRemaining ? totalBytesRemaining : bytesRequested;

  memcpy(chunk, ((unsigned char *)_params.responseBody()) + context->bytesSent(), bytesToSend);
  context->setBytesSent(context->bytesSent() + bytesToSend);
  return ESB_SUCCESS;
}

void HttpOriginHandler::endTransaction(HttpMultiplexer &stack, HttpServerStream &stream, State state) {
  HttpOriginContext *context = (HttpOriginContext *)stream.context();
  assert(context);
  ESB::Allocator &allocator = stream.allocator();

  context->~HttpOriginContext();
  allocator.deallocate(context);
  stream.setContext(NULL);

  switch (state) {
    case ES_HTTP_SERVER_HANDLER_BEGIN:
      assert(!"Transaction failed at begin state");
      ESB_LOG_ERROR("[%s] Transaction failed at begin state", stream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS:
      // Can fail here when the server connection is waiting for the next request
      // assert(!"Transaction failed at request header parse state");
      // ESB_LOG_ERROR("[%s] Transaction failed at request header parse state", stream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY:
      assert(!"Transaction failed at request body parse state");
      ESB_LOG_ERROR("[%s] Transaction failed at request body parse state", stream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS:
      assert(!"Transaction failed at response header send state");
      ESB_LOG_ERROR("[%s] Transaction failed at response header send state", stream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY:
      assert(!"Transaction failed at response body send state");
      ESB_LOG_ERROR("[%s] Transaction failed at response body send state", stream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_END:
#ifndef NDEBUG
      if (0 <= _params.requestSize()) {
        ESB::UInt32 bytesReceived = context->bytesReceived();
        assert(bytesReceived == _params.requestSize());
      }
#endif
      break;
    default:
      assert(!"Transaction failed at unknown state");
      ESB_LOG_ERROR("[%s] Transaction failed at unknown state %d", stream.logAddress(), state);
  }
}

}  // namespace ES
