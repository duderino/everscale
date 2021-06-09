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
  HttpLoadgenSeedCommand(const ESB::SocketAddress &destination, const HttpTestParams &params,
                         ESB::CleanupHandler &cleanupHandler);

  virtual ~HttpLoadgenSeedCommand();

  virtual ESB::Error run(HttpMultiplexerExtended &multiplexer);

  virtual const char *name() { return "start load"; }

  virtual ESB::CleanupHandler *cleanupHandler() { return &_cleanupHandler; }

 private:
  ESB::Error buildRequest(HttpClientTransaction *transaction);

  ESB::SocketAddress _destination;
  const HttpTestParams &_params;
  ESB::CleanupHandler &_cleanupHandler;

  ESB_DEFAULT_FUNCS(HttpLoadgenSeedCommand);
};

}  // namespace ES

#endif
