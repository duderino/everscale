#ifndef ES_HTTP_ECHO_CLIENT_SEED_COMMAND_H
#define ES_HTTP_ECHO_CLIENT_SEED_COMMAND_H

#ifndef ES_HTTP_CLIENT_COMMAND_H
#include <ESHttpClientCommand.h>
#endif

#ifndef ES_HTTP_ECHO_CLIENT_CONTEXT_H
#include <ESHttpEchoClientContext.h>
#endif

#ifndef ES_HTTP_ECHO_CLIENT_REQUEST_BUILDER_H
#include <ESHttpEchoClientRequestBuilder.h>
#endif

namespace ES {

class HttpEchoClientSeedCommand : public HttpClientCommand {
 public:
  HttpEchoClientSeedCommand(ESB::UInt32 connections, ESB::UInt32 iterations,
                            ESB::Int32 port, const char *host,
                            const char *absPath, const char *method,
                            const char *contentType,
                            ESB::CleanupHandler &cleanupHandler)
      : _connections(connections),
        _iterations(iterations),
        _port(port),
        _host(host),
        _absPath(absPath),
        _method(method),
        _contentType(contentType),
        _cleanupHandler(cleanupHandler) {}

  virtual ~HttpEchoClientSeedCommand() {}

  virtual ESB::Error run(HttpClientStack &stack) {
    for (ESB::UInt32 i = 0; i < _connections; ++i) {
      if (0 > HttpEchoClientContext::DecRemainingIterations()) {
        break;
      }

      HttpClientTransaction *transaction = stack.createClientTransaction();
      assert(transaction);

      // Create the request context
      HttpEchoClientContext *context =
          new (ESB::SystemAllocator::Instance()) HttpEchoClientContext(
              ESB::SystemAllocator::Instance().cleanupHandler());
      assert(context);

      transaction->setContext(context);

      // Build the request

      ESB::Error error = HttpEchoClientRequestBuilder(
          _host, _port, _absPath, _method, _contentType, transaction);
      assert(ESB_SUCCESS == error);

      error = stack.executeTransaction(transaction);
      assert(ESB_SUCCESS == error);
    }

    return ESB_SUCCESS;
  }

  virtual const char *name() { return "SeedCommand"; }

  virtual ESB::CleanupHandler *cleanupHandler() { return &_cleanupHandler; }

 private:
  // Disabled
  HttpEchoClientSeedCommand(const HttpEchoClientSeedCommand &);
  HttpEchoClientSeedCommand &operator=(const HttpEchoClientSeedCommand &);

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
