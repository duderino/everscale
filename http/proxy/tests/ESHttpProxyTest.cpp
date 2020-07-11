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

#ifndef ES_HTTP_ROUTING_PROXY_HANDLER_H
#include <ESHttpRoutingProxyHandler.h>
#endif

#ifndef ES_HTTP_ORIGIN_HANDLER_H
#include <ESHttpOriginHandler.h>
#endif

#ifndef ES_HTTP_FIXED_ROUTER_H
#include <ESHttpFixedRouter.h>
#endif

using namespace ES;

int main(int argc, char **argv) {
  HttpTestParams params;
  params.connections(500).iterations(500).clientThreads(3).proxyThreads(3).originThreads(3).logLevel(
      ESB::Logger::Notice);
  ESB::Error error = params.override(argc, argv);
  if (ESB_SUCCESS != error) {
    return error;
  }

  EphemeralListener originListener("origin-listener");
  EphemeralListener proxyListener("proxy-listener");
  HttpFixedRouter router(originListener.localDestination());
  HttpLoadgenHandler loadgenHandler(params);
  HttpRoutingProxyHandler proxyHandler(router);
  HttpOriginHandler originHandler(params);

  HttpIntegrationTest test(params, originListener, proxyListener, loadgenHandler, proxyHandler, originHandler);
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
