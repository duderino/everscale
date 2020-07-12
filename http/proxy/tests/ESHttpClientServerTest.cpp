#ifndef ES_HTTP_INTEGRATION_TEST_H
#include "ESHttpIntegrationTest.h"
#endif

#ifndef ES_HTTP_TEST_PARAMS_H
#include <ESHttpTestParams.h>
#endif

#ifndef ES_EPHEMERAL_LISTENER_H
#include <ESEphemeralListener.h>
#endif

#ifndef ES_HTTP_LOADGEN_HANDLER_H
#include <ESHttpLoadgenHandler.h>
#endif

#ifndef ES_HTTP_ORIGIN_HANDLER_H
#include <ESHttpOriginHandler.h>
#endif

using namespace ES;

int main(int argc, char **argv) {
  HttpTestParams params;
  params.connections(500)
      .iterations(500)
      .clientThreads(3)
      .originThreads(3)
      .requestSize(1024)
      .responseSize(1024)
      .logLevel(ESB::Logger::Notice);

  ESB::Error error = params.override(argc, argv);
  if (ESB_SUCCESS != error) {
    return error;
  }

  EphemeralListener listener("listener");
  HttpLoadgenHandler loadgenHandler(params);
  HttpOriginHandler originHandler(params);

  HttpIntegrationTest test(params, listener, loadgenHandler, originHandler);
  error = test.run();
  if (ESB_SUCCESS != error) {
    return error;
  }

  //
  // Assert all requests succeeded
  //

  const ESB::UInt32 totalSuccesses = test.clientCounters().getSuccesses()->queries();
  const ESB::UInt32 totalFailures = test.clientCounters().getFailures()->queries();
  const ESB::UInt32 totalTransactions = params.connections() * params.iterations();

  if (totalSuccesses != totalTransactions || 0 < totalFailures) {
    ESB_LOG_CRITICAL("TEST FAILURE: expected %u successes but got %u successes and %u failures", totalTransactions,
                     totalSuccesses, totalFailures);
    return ESB_OTHER_ERROR;
  }

  return ESB_SUCCESS;
}
