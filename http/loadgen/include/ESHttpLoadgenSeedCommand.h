#ifndef ES_HTTP_LOADGEN_SEED_COMMAND_H
#define ES_HTTP_LOADGEN_SEED_COMMAND_H

#ifndef ES_HTTP_CLIENT_COMMAND_H
#include <ESHttpClientCommand.h>
#endif

namespace ES {

class HttpLoadgenSeedCommand : public HttpClientCommand {
 public:
  HttpLoadgenSeedCommand(ESB::UInt32 connections, ESB::UInt32 iterations, ESB::SocketAddress &destination,
                         ESB::Int32 port, const char *host, const char *absPath, const char *method,
                         const char *contentType, ESB::CleanupHandler &cleanupHandler);

  virtual ~HttpLoadgenSeedCommand();

  virtual ESB::Error run(HttpMultiplexerExtended &multiplexer);

  virtual const char *name() { return "SeedCommand"; }

  virtual ESB::CleanupHandler *cleanupHandler() { return &_cleanupHandler; }

 private:
  // Disabled
  HttpLoadgenSeedCommand(const HttpLoadgenSeedCommand &);
  HttpLoadgenSeedCommand &operator=(const HttpLoadgenSeedCommand &);

  ESB::Error buildRequest(HttpClientTransaction *transaction);

  ESB::SocketAddress _destination;
  const ESB::UInt32 _connections;
  const ESB::UInt32 _iterations;
  const char *_host;
  const char *_absPath;
  const char *_method;
  const char *_contentType;
  ESB::CleanupHandler &_cleanupHandler;
};

}  // namespace ES

#endif
