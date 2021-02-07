#ifndef ES_HTTP_ORIGIN_HANDLER_H
#include <ESHttpOriginHandler.h>
#endif

#ifndef ES_HTTP_ORIGIN_CONTEXT_H
#include <ESHttpOriginContext.h>
#endif

namespace ES {

HttpOriginHandler::HttpOriginHandler(const HttpTestParams &params) : _params(params) {}

HttpOriginHandler::~HttpOriginHandler() {}

ESB::Error HttpOriginHandler::acceptConnection(HttpMultiplexer &stack, ESB::SocketAddress *address) {
  return ESB_SUCCESS;
}

ESB::Error HttpOriginHandler::beginTransaction(HttpMultiplexer &stack, HttpServerStream &stream) {
  switch (_params.disruptTransaction()) {
    case HttpTestParams::STALL_SERVER_RECV_HEADERS:
      stream.pauseSend(false);
      stream.pauseRecv(false);
      return ESB_PAUSE;
    case HttpTestParams::CLOSE_SERVER_RECV_HEADERS:
      return ESB_CLOSED;
    case HttpTestParams::HAPPY_PATH:
    default:
      break;
  }

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
                                                 const unsigned char *chunk, ESB::UInt32 chunkSize,
                                                 ESB::UInt32 *bytesConsumed) {
  switch (_params.disruptTransaction()) {
    case HttpTestParams::STALL_SERVER_RECV_BODY:
      stream.pauseSend(false);
      stream.pauseRecv(false);
      return ESB_PAUSE;
    case HttpTestParams::CLOSE_SERVER_RECV_BODY:
      return ESB_CLOSED;
    case HttpTestParams::HAPPY_PATH:
    default:
      break;
  }

  assert(chunk);
  assert(bytesConsumed);
  HttpOriginContext *context = (HttpOriginContext *)stream.context();
  assert(context);

#ifndef NDEBUG
  for (int i = 0; i < chunkSize; ++i) {
    char expected = 'a' + (context->bytesReceived() + i) % 26;
    if (expected != chunk[i]) {
      assert(chunk[i] == expected);
    }
  }
#endif

  *bytesConsumed = chunkSize;
  context->setBytesReceived(context->bytesReceived() + chunkSize);

  ESB_LOG_DEBUG("[%s] received %u or %u/%u request body bytes", stream.logAddress(), chunkSize,
                context->bytesReceived(), _params.requestSize());

  HttpResponse &response = stream.response();

  if (0 < chunkSize) {
    return ESB_SUCCESS;
  }

  if (0 <= _params.requestSize() && _params.requestSize() != context->bytesReceived()) {
    ESB_LOG_ERROR("[%s] missing %u bytes from %u byte response body", stream.logAddress(),
                  _params.requestSize() - context->bytesReceived(), _params.requestSize());
  }
  assert(context->bytesReceived() == _params.requestSize());

  switch (_params.disruptTransaction()) {
    case HttpTestParams::STALL_SERVER_SEND_HEADERS:
      stream.pauseSend(false);
      stream.pauseRecv(false);
      return ESB_PAUSE;
    case HttpTestParams::CLOSE_SERVER_SEND_HEADERS:
      return ESB_CLOSED;
    case HttpTestParams::HAPPY_PATH:
    default:
      break;
  }

  response.setStatusCode(200);
  response.setReasonPhrase("OK");

  ESB::Error error;

  if (_params.useContentLengthHeader()) {
    error = response.addHeader(stream.allocator(), "Content-Length", "%u", _params.requestSize());
  } else {
    error = response.addHeader("Transfer-Encoding", "chunked", stream.allocator());
  }

  if (ESB_SUCCESS != error) {
    return error;
  }

  if (_params.contentType()) {
    error = response.addHeader("Content-Type", _params.contentType(), stream.allocator());
    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  return ESB_SEND_RESPONSE;
}

ESB::Error HttpOriginHandler::offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                                ESB::UInt32 *bytesAvailable) {
  switch (_params.disruptTransaction()) {
    case HttpTestParams::STALL_SERVER_SEND_BODY:
      stream.pauseSend(false);
      stream.pauseRecv(false);
      return ESB_PAUSE;
    case HttpTestParams::CLOSE_SERVER_SEND_BODY:
      return ESB_CLOSED;
    case HttpTestParams::HAPPY_PATH:
    default:
      break;
  }

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

  ESB_LOG_DEBUG("[%s] sent %u or %u/%u response body bytes", stream.logAddress(), bytesToSend, context->bytesSent(),
                _params.responseSize());

  return ESB_SUCCESS;
}

void HttpOriginHandler::endTransaction(HttpMultiplexer &stack, HttpServerStream &stream, State state) {
  switch (state) {
    case ES_HTTP_SERVER_HANDLER_BEGIN:
      ESB_LOG_ERROR("[%s] Transaction failed at begin state", stream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS:
      // Can fail here when the server connection is waiting for the next request
      // assert(!"Transaction failed at request header parse state");
      // ESB_LOG_ERROR("[%s] Transaction failed at request header parse state", stream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY:
      ESB_LOG_ERROR("[%s] Transaction failed at request body parse state", stream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS:
      ESB_LOG_ERROR("[%s] Transaction failed at response header send state", stream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY:
      ESB_LOG_ERROR("[%s] Transaction failed at response body send state", stream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_END:
      break;
    default:
      ESB_LOG_ERROR("[%s] Transaction failed at unknown state %d", stream.logAddress(), state);
  }

  HttpOriginContext *context = (HttpOriginContext *)stream.context();
  if (!context) {
    return;
  }
  ESB::Allocator &allocator = stream.allocator();

  context->~HttpOriginContext();
  allocator.deallocate(context);
  stream.setContext(NULL);
}

}  // namespace ES
