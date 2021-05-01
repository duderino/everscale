#ifndef ES_HTTP_LOADGEN_SEED_COMMAND_H
#include <ESHttpLoadgenSeedCommand.h>
#endif

#ifndef ES_HTTP_LOADGEN_CONTEXT_H
#include <ESHttpLoadgenContext.h>
#endif

namespace ES {

HttpLoadgenSeedCommand::HttpLoadgenSeedCommand(const ESB::SocketAddress &destination, const HttpTestParams &params,
                                               ESB::CleanupHandler &cleanupHandler)
    : _destination(destination), _params(params), _cleanupHandler(cleanupHandler) {}

HttpLoadgenSeedCommand::~HttpLoadgenSeedCommand() {}

ESB::Error HttpLoadgenSeedCommand::run(HttpMultiplexerExtended &multiplexer) {
  for (ESB::UInt32 i = 0; i < _params.connections() / _params.clientThreads(); ++i) {
    if (0 > HttpLoadgenContext::DecRemainingIterations()) {
      break;
    }

    HttpClientTransaction *transaction = multiplexer.createClientTransaction();
    assert(transaction);

    // Create the request context
    HttpLoadgenContext *context =
        new (ESB::SystemAllocator::Instance()) HttpLoadgenContext(ESB::SystemAllocator::Instance().cleanupHandler());
    if (!context) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "Cannot create loadgen context");
      return ESB_OUT_OF_MEMORY;
    }

    // Build the request

    ESB::Error error = buildRequest(transaction);
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot build initial request");
      return error;
    }

    transaction->setContext(context);
    transaction->setPeerAddress(_destination);

    if (_params.secure()) {
      assert(ESB::SocketAddress::TLS == transaction->peerAddress().type());
      transaction->request().requestUri().setType(HttpRequestUri::ES_URI_HTTPS);
    } else {
      assert(ESB::SocketAddress::TCP == transaction->peerAddress().type());
      transaction->request().requestUri().setType(HttpRequestUri::ES_URI_HTTP);
    }

    error = multiplexer.executeClientTransaction(transaction);
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot execute client transaction");
      return error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpLoadgenSeedCommand::buildRequest(HttpClientTransaction *transaction) {
  HttpRequest &request = transaction->request();
  HttpRequestUri &requestUri = request.requestUri();

  requestUri.setType(HttpRequestUri::ES_URI_HTTP);
  requestUri.setAbsPath(_params.absPath());
  request.setMethod(_params.method());

  ESB::Error error =
      request.addHeader(transaction->allocator(), "Host", "%s:%d", _params.hostHeader(), _destination.port());

  if (ESB_SUCCESS != error) {
    return error;
  }

  if (_params.contentType()) {
    error = request.addHeader("Content-Type", _params.contentType(), transaction->allocator());
    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  if (_params.useContentLengthHeader()) {
    error = request.addHeader(transaction->allocator(), "Content-Length", "%lu", _params.requestSize());
  } else {
    error = request.addHeader("Transfer-Encoding", "chunked", transaction->allocator());
  }

  return error;
}

}  // namespace ES
