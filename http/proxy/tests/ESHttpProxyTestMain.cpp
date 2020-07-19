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
#include "ESHttpFixedRouter.h"
#endif

using namespace ES;

/**
 * Test case suitable for command-line usage and lightweight load testing
 *
 * Example of client -> proxy -> server:
 *
 * ./http/proxy/http-proxy-test-main -c 3 -p 3 -o 3 -s 500 -i 5000 -r 1
 *
 * 3 client threads (-c 3) sending to 3 proxy threads (-p3) sending to three origin server threads (-o 3)
 * 500 concurrent connections (-s 500)
 * 5000 requests per connection (-i 5000)
 * reuse connections in both client and proxy (-r 1)
 *
 * Example of client -> server (good for distinguishing stack issues from proxy errors):
 *
 * ./http/proxy/http-proxy-test-main -c 3 -p 0 -o 3 -s 500 -i 5000 -r 1
 *
 * 3 client threads (-c 3) sending to three origin server threads (-o 3)
 * 0 proxy threads (-p 0) configures the client threads to send directly to origin threads.
 * 500 concurrent connections (-s 500)
 * 5000 requests per connection (-i 5000)
 * reuse connections in both client and proxy (-r 1)
 */
int main(int argc, char **argv) {
  HttpTestParams params;
  params.connections(500)
      .iterations(500)
      .clientThreads(3)
      .proxyThreads(3)
      .originThreads(3)
      .requestSize(1024)
      .responseSize(1024)
      .logLevel(ESB::Logger::Notice);
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
