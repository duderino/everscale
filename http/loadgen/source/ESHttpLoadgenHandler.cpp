#ifndef ES_HTTP_LOADGEN_HANDLER_H
#include <ESHttpLoadgenHandler.h>
#endif

#ifndef ES_HTTP_LOADGEN_CONTEXT_H
#include <ESHttpLoadgenContext.h>
#endif

namespace ES {

HttpLoadgenHandler::HttpLoadgenHandler(const HttpTestParams &params) : _params(params), _completedTransactions() {}

HttpLoadgenHandler::~HttpLoadgenHandler() {}

ESB::Error HttpLoadgenHandler::beginTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) {
  return ESB_SUCCESS;
}

ESB::Error HttpLoadgenHandler::offerRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &stream,
                                                ESB::UInt32 *maxChunkSize) {
  assert(maxChunkSize);
  HttpLoadgenContext *context = (HttpLoadgenContext *)stream.context();
  assert(context);
  *maxChunkSize = _params.requestSize() - context->bytesSent();
  return ESB_SUCCESS;
}

ESB::Error HttpLoadgenHandler::produceRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &stream,
                                                  unsigned char *chunk, ESB::UInt32 bytesRequested) {
  assert(chunk);
  assert(0 < bytesRequested);
  HttpLoadgenContext *context = (HttpLoadgenContext *)stream.context();
  assert(context);
  assert(bytesRequested <= _params.requestSize() - context->bytesSent());

  ESB::UInt32 totalBytesRemaining = _params.requestSize() - context->bytesSent();
  ESB::UInt32 bytesToSend = bytesRequested > totalBytesRemaining ? totalBytesRemaining : bytesRequested;

  memcpy(chunk, _params.requestBody() + context->bytesSent(), bytesToSend);
  context->setBytesSent(context->bytesSent() + bytesToSend);

  ESB_LOG_DEBUG("[%s] sent %u or %u/%u request body bytes", stream.logAddress(), bytesToSend, context->bytesSent(),
                _params.requestSize());

  return ESB_SUCCESS;
}

ESB::Error HttpLoadgenHandler::receiveResponseHeaders(HttpMultiplexer &multiplexer, HttpClientStream &stream) {
  return ESB_SUCCESS;
}

ESB::Error HttpLoadgenHandler::consumeResponseBody(HttpMultiplexer &multiplexer, HttpClientStream &stream,
                                                   const unsigned char *chunk, ESB::UInt32 chunkSize,
                                                   ESB::UInt32 *bytesConsumed) {
  assert(chunk);
  assert(bytesConsumed);
  HttpLoadgenContext *context = (HttpLoadgenContext *)stream.context();
  assert(context);

#ifndef NDEBUG
  for (int i = 0; i < chunkSize; ++i) {
    char expected = 'A' + (context->bytesReceived() + i) % 26;
    if (expected != chunk[i]) {
      assert(chunk[i] == expected);
    }
  }
#endif

  *bytesConsumed = chunkSize;
  context->setBytesRecieved(context->bytesReceived() + chunkSize);

  ESB_LOG_DEBUG("[%s] received %u or %u/%u response body bytes", stream.logAddress(), chunkSize,
                context->bytesReceived(), _params.responseSize());

  return ESB_SUCCESS;
}

void HttpLoadgenHandler::endTransaction(HttpMultiplexer &multiplexer, HttpClientStream &stream, State state) {
  HttpLoadgenContext *context = (HttpLoadgenContext *)stream.context();
  assert(context);

  switch (state) {
    case ES_HTTP_CLIENT_HANDLER_BEGIN:
      ESB_LOG_ERROR("[%s] transaction failed at begin state", stream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_RESOLVE:
      ESB_LOG_ERROR("[%s] transaction failed at resolve state", stream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_CONNECT:
      ESB_LOG_ERROR("[%s] transaction failed at connect state", stream.logAddress());
    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS:
      ESB_LOG_ERROR("[%s] transaction failed at send request headers state", stream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY:
      ESB_LOG_ERROR("[%s] transaction failed at send request body state", stream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS:
      ESB_LOG_ERROR("[%s] transaction failed at receive response headers state", stream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY:
      ESB_LOG_ERROR("[%s] transaction failed at receive response body state", stream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_END:
      if (0 <= _params.responseSize() && _params.responseSize() != context->bytesReceived()) {
        ESB_LOG_ERROR("[%s] missing %u bytes from %u byte response body", stream.logAddress(),
                      _params.responseSize() - context->bytesReceived(), _params.responseSize());
      }
      assert(context->bytesReceived() == _params.responseSize());
      break;
    default:
      assert(!"Transaction failed at unknown state");
      ESB_LOG_ERROR("[%s] Transaction failed at unknown state %d", stream.logAddress(), state);
  }

  HttpLoadgenContext::IncCompletedIterations();
  // returns the value pre-decrement
  int remainingIterations = HttpLoadgenContext::DecRemainingIterations();

  if (0 > remainingIterations) {
    ESB::CleanupHandler &cleanupHandler = context->cleanupHandler();
    cleanupHandler.destroy(context);
    stream.setContext(NULL);
    return;
  }

  HttpClientTransaction *newTransaction = multiplexer.createClientTransaction();

  if (!newTransaction) {
    ESB_LOG_WARNING_ERRNO(ESB_OUT_OF_MEMORY, "Cannot create new transaction");
    return;
  }

  ESB::Error error = newTransaction->request().copy(&stream.request(), newTransaction->allocator());
  if (ESB_SUCCESS != error) {
    multiplexer.destroyClientTransaction(newTransaction);
    ESB_LOG_WARNING_ERRNO(error, "Cannot build request");
    return;
  }

  newTransaction->setPeerAddress(stream.peerAddress());

  context->setBytesSent(0U);
  context->setBytesRecieved(0U);
  newTransaction->setContext(context);
  stream.setContext(NULL);

  error = multiplexer.executeClientTransaction(newTransaction);

  if (ESB_SUCCESS != error) {
    multiplexer.destroyClientTransaction(newTransaction);
    ESB_LOG_WARNING_ERRNO(error, "Cannot execute transaction");
    return;
  }

  ESB_LOG_DEBUG("Resubmitted transaction.  %u iterations remaining", remainingIterations);
}

ESB::Error HttpLoadgenHandler::endRequest(HttpMultiplexer &multiplexer, HttpClientStream &stream) {
  return ESB_SUCCESS;
}

}  // namespace ES
