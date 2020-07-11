#ifndef ES_HTTP_LOADGEN_HANDLER_H
#include <ESHttpLoadgenHandler.h>
#endif

#ifndef ES_HTTP_LOADGEN_CONTEXT_H
#include <ESHttpLoadgenContext.h>
#endif

namespace ES {

HttpLoadgenHandler::HttpLoadgenHandler(HttpTestParams &params) : _params(params), _completedTransactions() {}

HttpLoadgenHandler::~HttpLoadgenHandler() {}

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

  *bytesConsumed = chunkSize;
  context->setBytesRecieved(context->bytesReceived() + chunkSize);

  return ESB_SUCCESS;
}

void HttpLoadgenHandler::endTransaction(HttpMultiplexer &multiplexer, HttpClientStream &stream, State state) {
  HttpLoadgenContext *context = (HttpLoadgenContext *)stream.context();
  assert(context);

  switch (state) {
    case ES_HTTP_CLIENT_HANDLER_BEGIN:
      assert(!"Transaction failed at begin state");
      ESB_LOG_ERROR("Transaction failed at begin state");
      break;
    case ES_HTTP_CLIENT_HANDLER_RESOLVE:
      assert(!"Transaction failed at resolve state");
      ESB_LOG_ERROR("Transaction failed at resolve state");
      break;
    case ES_HTTP_CLIENT_HANDLER_CONNECT:
      assert(!"Transaction failed at connect state");
      ESB_LOG_ERROR("Transaction failed at connect state");
    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS:
      assert(!"Transaction failed at send request headers state");
      ESB_LOG_ERROR("Transaction failed at send request headers state");
      break;
    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY:
      assert(!"Transaction failed at send request body state");
      ESB_LOG_ERROR("Transaction failed at send request body state");
      break;
    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS:
      assert(!"Transaction failed at receive response headers state");
      ESB_LOG_ERROR("Transaction failed at receive response headers state");
      break;
    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY:
      assert(!"Transaction failed at receive response body state");
      ESB_LOG_ERROR("Transaction failed at receive response body state");
      break;
    case ES_HTTP_CLIENT_HANDLER_END:
#ifndef NDEBUG
      if (0 <= _params.responseSize()) {
        ESB::UInt32 bytesReceived = context->bytesReceived();
        assert(bytesReceived == _params.responseSize());
      }
#endif
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
