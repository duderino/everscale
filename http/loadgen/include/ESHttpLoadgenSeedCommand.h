#ifndef ES_HTTP_LOADGEN_SEED_COMMAND_H
#define ES_HTTP_LOADGEN_SEED_COMMAND_H

#ifndef ES_HTTP_CLIENT_COMMAND_H
#include <ESHttpClientCommand.h>
#endif

#ifndef ES_HTTP_LOADGEN_CONTEXT_H
#include <ESHttpLoadgenContext.h>
#endif

#ifndef ES_HTTP_LOADGEN_REQUEST_BUILDER_H
#include <ESHttpLoadgenRequestBuilder.h>
#endif

namespace ES {

class HttpLoadgenSeedCommand : public HttpClientCommand {
 public:
  HttpLoadgenSeedCommand(ESB::UInt32 connections, ESB::UInt32 iterations,
                         ESB::Int32 port, const char *host, const char *absPath,
                         const char *method, const char *contentType,
                         ESB::CleanupHandler &cleanupHandler)
      : _connections(connections),
        _iterations(iterations),
        _port(port),
        _host(host),
        _absPath(absPath),
        _method(method),
        _contentType(contentType),
        _cleanupHandler(cleanupHandler) {}

  virtual ~HttpLoadgenSeedCommand() {}

  virtual ESB::Error run(HttpMultiplexer &multiplexer) {
    for (ESB::UInt32 i = 0; i < _connections; ++i) {
      if (0 > HttpLoadgenContext::DecRemainingIterations()) {
        break;
      }

      HttpClientTransaction *transaction =
          multiplexer.createClientTransaction();
      assert(transaction);

      // Create the request context
      HttpLoadgenContext *context = new (ESB::SystemAllocator::Instance())
          HttpLoadgenContext(ESB::SystemAllocator::Instance().cleanupHandler());
      assert(context);

      transaction->setContext(context);

      // Build the request

      ESB::Error error = HttpLoadgenRequestBuilder(
          _host, _port, _absPath, _method, _contentType, transaction);
      assert(ESB_SUCCESS == error);

      error = multiplexer.executeTransaction(transaction);
      assert(ESB_SUCCESS == error);
    }

    return ESB_SUCCESS;
  }

  virtual const char *name() { return "SeedCommand"; }

  virtual ESB::CleanupHandler *cleanupHandler() { return &_cleanupHandler; }

 private:
  // Disabled
  HttpLoadgenSeedCommand(const HttpLoadgenSeedCommand &);
  HttpLoadgenSeedCommand &operator=(const HttpLoadgenSeedCommand &);

  const ESB::UInt32 _connections;
  const ESB::UInt32 _iterations;
  const ESB::Int32 _port;
  const char *_host;
  const char *_absPath;
  const char *_method;
  const char *_contentType;
  ESB::CleanupHandler &_cleanupHandler;
};

}  // namespace ES

#endif
