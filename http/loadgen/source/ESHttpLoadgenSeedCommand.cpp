#ifndef ES_HTTP_LOADGEN_SEED_COMMAND_H
#include <ESHttpLoadgenSeedCommand.h>
#endif

#ifndef ES_HTTP_LOADGEN_CONTEXT_H
#include <ESHttpLoadgenContext.h>
#endif

namespace ES {
HttpLoadgenSeedCommand::HttpLoadgenSeedCommand(
    ESB::UInt32 connections, ESB::UInt32 iterations,
    ESB::SocketAddress &destination, ESB::Int32 port, const char *host,
    const char *absPath, const char *method, const char *contentType,
    ESB::CleanupHandler &cleanupHandler)
    : _destination(destination),
      _connections(connections),
      _iterations(iterations),
      _host(host),
      _absPath(absPath),
      _method(method),
      _contentType(contentType),
      _cleanupHandler(cleanupHandler) {}

HttpLoadgenSeedCommand::~HttpLoadgenSeedCommand() {}

ESB::Error HttpLoadgenSeedCommand::run(HttpMultiplexerExtended &multiplexer) {
  for (ESB::UInt32 i = 0; i < _connections; ++i) {
    if (0 > HttpLoadgenContext::DecRemainingIterations()) {
      break;
    }

    HttpClientTransaction *transaction = multiplexer.createClientTransaction();
    assert(transaction);

    // Create the request context
    HttpLoadgenContext *context = new (ESB::SystemAllocator::Instance())
        HttpLoadgenContext(ESB::SystemAllocator::Instance().cleanupHandler());
    assert(context);

    // Build the request

    ESB::Error error = buildRequest(transaction);
    assert(ESB_SUCCESS == error);
    transaction->setContext(context);
    transaction->setPeerAddress(_destination);

    error = multiplexer.executeClientTransaction(transaction);
    assert(ESB_SUCCESS == error);
  }

  return ESB_SUCCESS;
}

ESB::Error HttpLoadgenSeedCommand::buildRequest(
    HttpClientTransaction *transaction) {
  HttpRequest &request = transaction->request();
  HttpRequestUri &requestUri = request.requestUri();

  requestUri.setType(HttpRequestUri::ES_URI_HTTP);
  requestUri.setAbsPath(_absPath);
  request.setMethod(_method);

  ESB::Error error = request.addHeader(transaction->allocator(), "Host",
                                       "%s:%d", _host, _destination.port());

  if (ESB_SUCCESS != error) {
    return error;
  }

  if (_contentType) {
    error = request.addHeader("Content-Type", _contentType,
                              transaction->allocator());
    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  error = request.addHeader("Transfer-Encoding", "chunked",
                            transaction->allocator());

  if (ESB_SUCCESS != error) {
    return error;
  }

  return ESB_SUCCESS;
}

}  // namespace ES
