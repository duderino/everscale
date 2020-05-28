#ifndef ES_HTTP_LOADGEN_HANDLER_H
#include <ESHttpLoadgenHandler.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ES_HTTP_LOADGEN_CONTEXT_H
#include <ESHttpLoadgenContext.h>
#endif

#ifndef ES_HTTP_LOADGEN_REQUEST_BUILDER_H
#include <ESHttpLoadgenRequestBuilder.h>
#endif

namespace ES {

HttpLoadgenHandler::HttpLoadgenHandler(const char *absPath, const char *method,
                                       const char *contentType,
                                       const unsigned char *body, int bodySize)
    : _absPath(absPath),
      _method(method),
      _contentType(contentType),
      _body(body),
      _bodySize(bodySize),
      _completedTransactions() {}

HttpLoadgenHandler::~HttpLoadgenHandler() {}

ESB::Error HttpLoadgenHandler::offerRequestBody(HttpMultiplexer &multiplexer,
                                                HttpClientStream &stream,
                                                ESB::UInt32 *maxChunkSize) {
  assert(maxChunkSize);
  HttpLoadgenContext *context = (HttpLoadgenContext *)stream.context();
  assert(context);
  *maxChunkSize = _bodySize - context->bytesSent();
  return ESB_SUCCESS;
}

ESB::Error HttpLoadgenHandler::produceRequestBody(HttpMultiplexer &multiplexer,
                                                  HttpClientStream &stream,
                                                  unsigned char *chunk,
                                                  ESB::UInt32 bytesRequested) {
  assert(chunk);
  assert(0 < bytesRequested);
  HttpLoadgenContext *context = (HttpLoadgenContext *)stream.context();
  assert(context);
  assert(bytesRequested <= _bodySize - context->bytesSent());

  ESB::UInt32 totalBytesRemaining = _bodySize - context->bytesSent();
  ESB::UInt32 bytesToSend = bytesRequested > totalBytesRemaining
                                ? totalBytesRemaining
                                : bytesRequested;

  memcpy(chunk, _body + context->bytesSent(), bytesToSend);
  context->setBytesSent(context->bytesSent() + bytesToSend);
  return ESB_SUCCESS;
}

ESB::Error HttpLoadgenHandler::receiveResponseHeaders(
    HttpMultiplexer &multiplexer, HttpClientStream &stream) {
  return ESB_SUCCESS;
}

ESB::Error HttpLoadgenHandler::consumeResponseBody(HttpMultiplexer &multiplexer,
                                                   HttpClientStream &stream,
                                                   const unsigned char *chunk,
                                                   ESB::UInt32 chunkSize,
                                                   ESB::UInt32 *bytesConsumed) {
  assert(chunk);
  assert(bytesConsumed);

  *bytesConsumed = chunkSize;
  return ESB_SUCCESS;
}

void HttpLoadgenHandler::endTransaction(HttpMultiplexer &multiplexer,
                                        HttpClientStream &stream, State state) {
  HttpLoadgenContext *context = (HttpLoadgenContext *)stream.context();
  assert(context);

  switch (state) {
    case ES_HTTP_CLIENT_HANDLER_BEGIN:
      ESB_LOG_INFO("Transaction failed at begin state");
      break;
    case ES_HTTP_CLIENT_HANDLER_RESOLVE:
      ESB_LOG_INFO("Transaction failed at resolve state");
      break;
    case ES_HTTP_CLIENT_HANDLER_CONNECT:
      ESB_LOG_INFO("Transaction failed at connect state");
    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS:
      ESB_LOG_INFO("Transaction failed at send request headers state");
      break;
    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY:
      ESB_LOG_INFO("Transaction failed at send request body state");
      break;
    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS:
      ESB_LOG_INFO("Transaction failed at receive response headers state");
      break;
    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY:
      ESB_LOG_INFO("Transaction failed at receive response body state");
      break;
    case ES_HTTP_CLIENT_HANDLER_END:
      break;
    default:
      ESB_LOG_WARNING("Transaction failed at unknown state");
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

  context->setBytesSent(0U);

  newTransaction->setContext(context);
  stream.setContext(NULL);

  char dottedIP[ESB_IPV6_PRESENTATION_SIZE];
  stream.peerAddress().presentationAddress(dottedIP, sizeof(dottedIP));

  ESB::Error error =
      HttpLoadgenRequestBuilder(dottedIP, stream.peerAddress().port(), _absPath,
                                _method, _contentType, newTransaction);

  if (ESB_SUCCESS != error) {
    multiplexer.destroyClientTransaction(newTransaction);
    ESB_LOG_WARNING_ERRNO(error, "Cannot build request");
    return;
  }

  error = multiplexer.executeClientTransaction(newTransaction);

  if (ESB_SUCCESS != error) {
    multiplexer.destroyClientTransaction(newTransaction);
    ESB_LOG_WARNING_ERRNO(error, "Cannot execute transaction");
    return;
  }

  ESB_LOG_DEBUG("Resubmitted transaction.  %u iterations remaining",
                remainingIterations);
}

ESB::Error HttpLoadgenHandler::endRequest(HttpMultiplexer &multiplexer,
                                          HttpClientStream &stream) {
  return ESB_SUCCESS;
}

}  // namespace ES
