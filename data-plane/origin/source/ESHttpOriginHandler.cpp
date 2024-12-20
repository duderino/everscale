#ifndef ES_HTTP_ORIGIN_HANDLER_H
#include <ESHttpOriginHandler.h>
#endif

#ifndef ES_HTTP_ORIGIN_CONTEXT_H
#include <ESHttpOriginContext.h>
#endif

namespace ES {

HttpOriginHandler::HttpOriginHandler(const HttpTestParams &params) : _params(params), _serverCounters() {}

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
                                                 const unsigned char *chunk, ESB::UInt64 chunkSize,
                                                 ESB::UInt64 *bytesConsumed) {
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
  assert(chunkSize + context->bytesReceived() <= _params.requestSize());

#ifndef NDEBUG
  {
    const ESB::UInt64 bytesReceived = context->bytesReceived();
    for (ESB::UInt64 i = 0; i < chunkSize; ++i) {
      char actual = chunk[i];
      char expected = 'a' + (bytesReceived + i) % 26;
      assert(actual == expected);
    }
  }
#endif

  *bytesConsumed = chunkSize;
  context->setBytesReceived(context->bytesReceived() + chunkSize);

  ESB_LOG_DEBUG("[%s] received %lu or %lu/%lu request body bytes", stream.logAddress(), chunkSize,
                context->bytesReceived(), _params.requestSize());

  HttpResponse &response = stream.response();

  if (0 < chunkSize) {
    return ESB_SUCCESS;
  }

  if (0 <= _params.requestSize() && _params.requestSize() != context->bytesReceived()) {
    ESB_LOG_ERROR("[%s] missing %lu bytes from %lu byte request body", stream.logAddress(),
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
    error = response.addHeader(stream.allocator(), "Content-Length", "%lu", _params.requestSize());
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
                                                ESB::UInt64 *bytesAvailable) {
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

  assert(bytesAvailable);
  HttpOriginContext *context = (HttpOriginContext *)stream.context();
  assert(context);
  assert(context->bytesSent() <= _params.responseSize());
  *bytesAvailable = _params.responseSize() - context->bytesSent();
  return ESB_SUCCESS;
}

ESB::Error HttpOriginHandler::produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                                  unsigned char *chunk, ESB::UInt64 bytesRequested) {
  assert(chunk);
  assert(0 < bytesRequested);
  HttpOriginContext *context = (HttpOriginContext *)stream.context();
  assert(context);

  const ESB::UInt64 bytesSent = context->bytesSent();
  assert(bytesSent <= _params.responseSize());
  assert(bytesRequested <= _params.responseSize() - bytesSent);

#ifndef NDEBUG
  // Verified in HttpLoadgenHandler::consumeResponseBody
  for (ESB::UInt64 i = 0; i < bytesRequested; ++i) {
    chunk[i] = 'A' + (bytesSent + i) % 26;
  }
#endif

  context->setBytesSent(bytesSent + bytesRequested);

  ESB_LOG_DEBUG("[%s] sent %lu or %lu/%lu response body bytes", stream.logAddress(), bytesRequested,
                context->bytesSent(), _params.responseSize());

  return ESB_SUCCESS;
}

void HttpOriginHandler::endTransaction(HttpMultiplexer &stack, HttpServerStream &stream, State state) {
  HttpOriginContext *context = (HttpOriginContext *) stream.context();
  const ESB::Date &start = stream.transactionStartTime();
  const ESB::Date stop = ESB::Time::Instance().now();

  switch (state) {
    case ES_HTTP_SERVER_HANDLER_BEGIN:
      _serverCounters.requestHeaderBeginError().record(start, stop);
      ESB_LOG_INFO("[%s] Transaction failed at begin state", stream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS:
      // Can fail here when the server connection is waiting for the next request
      // assert(!"Transaction failed at request header parse state");
      _serverCounters.requestHeaderReceiveError().record(start, stop);
      ESB_LOG_DEBUG("[%s] Transaction failed at request header parse state", stream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY:
      _serverCounters.requestBodyReceiveError().record(start, stop);
      ESB_LOG_INFO("[%s] Transaction failed at request body parse state", stream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS:
      _serverCounters.responseHeaderSendError().record(start, stop);
      ESB_LOG_INFO("[%s] Transaction failed at response header send state", stream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY:
      _serverCounters.responseBodySendError().record(start, stop);
      ESB_LOG_INFO("[%s] Transaction failed at response body send state", stream.logAddress());
      break;
    case ES_HTTP_SERVER_HANDLER_END:
      _serverCounters.successfulTransactions().record(start, stop);
      _serverCounters.incrementStatusCounter(stream.response().statusCode(), start, stop);
      if (context) {
        if (_params.requestSize() != context->bytesReceived()) {
          _serverCounters.requestBodySizeError().record(start, stop);
          ESB_LOG_ERROR("[%s] expected %lu request body bytes but received %lu bytes (delta %lu)", stream.logAddress(),
                        _params.requestSize(), context->bytesReceived(),
                        _params.requestSize() - context->bytesReceived());
        }
        assert(context->bytesReceived() == _params.requestSize());
      }
      break;
    default:
      assert(!"Transaction failed at unknown state");
      _serverCounters.failedTransactions().record(start, stop);
      ESB_LOG_ERROR("[%s] Transaction failed at unknown state %d", stream.logAddress(), state);
  }

  if (!context) {
    return;
  }

  ESB::Allocator &allocator = stream.allocator();

  context->~HttpOriginContext();
  allocator.deallocate(context);
  stream.setContext(NULL);
}

void HttpOriginHandler::dumpServerCounters(ESB::Logger &logger, ESB::Logger::Severity severity) const {
  _serverCounters.log(logger, severity);
}

}  // namespace ES
