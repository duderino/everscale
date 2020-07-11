#ifndef ES_HTTP_LOADGEN_SEED_COMMAND_H
#define ES_HTTP_LOADGEN_SEED_COMMAND_H

#ifndef ES_HTTP_CLIENT_COMMAND_H
#include <ESHttpClientCommand.h>
#endif

#ifndef ES_HTTP_TEST_PARAMS_H
#include <ESHttpTestParams.h>
#endif

namespace ES {

class HttpLoadgenSeedCommand : public HttpClientCommand {
 public:
  HttpLoadgenSeedCommand(ESB::SocketAddress &destination, HttpTestParams &params, ESB::CleanupHandler &cleanupHandler);

  virtual ~HttpLoadgenSeedCommand();

  virtual ESB::Error run(HttpMultiplexerExtended &multiplexer);

  virtual const char *name() { return "start load"; }

  virtual ESB::CleanupHandler *cleanupHandler() { return &_cleanupHandler; }

 private:
  // Disabled
  HttpLoadgenSeedCommand(const HttpLoadgenSeedCommand &);
  HttpLoadgenSeedCommand &operator=(const HttpLoadgenSeedCommand &);

  ESB::Error buildRequest(HttpClientTransaction *transaction);

  ESB::SocketAddress _destination;
  HttpTestParams &_params;
  ESB::CleanupHandler &_cleanupHandler;
};

}  // namespace ES

#endif
