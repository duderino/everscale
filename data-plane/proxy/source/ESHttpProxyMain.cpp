#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
#endif

#ifndef ES_HTTP_TEST_PARAMS_H
#include <ESHttpTestParams.h>
#endif

#ifndef ES_HTTP_ROUTING_PROXY_HANDLER_H
#include <ESHttpRoutingProxyHandler.h>
#endif

#ifndef ES_HTTP_PROXY_H
#include <ESHttpProxy.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ESB_TIME_SOURCE_CACHE_H
#include <ESBTimeSourceCache.h>
#endif

#ifndef ESB_SIGNAL_HANDLER_H
#include <ESBSignalHandler.h>
#endif

#ifndef ES_HTTP_FIXED_ROUTER_H
#include "ESHttpFixedRouter.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

using namespace ES;

int main(int argc, char **argv) {
  HttpTestParams params;
  params.logLevel(ESB::Logger::Notice);
  ESB::Error error = params.override(argc, argv);
  if (ESB_SUCCESS != error) {
    return error;
  }

  if (0 == params.proxyPort()) {
    fprintf(stderr, "--proxyPort is required\n");
    return 1;
  }

  if (0 == params.originPort()) {
    fprintf(stderr, "--originPort is required\n");
    return 1;
  }

  if (params.proxyPort() == params.originPort()) {
    fprintf(stderr, "--proxyPort and --originPort must be different\n");
    return 1;
  }

  ESB::TimeSourceCache timeCache(ESB::SystemTimeSource::Instance());
  error = timeCache.start();
  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot start time cache thread");
    return error;
  }

  ESB::SimpleFileLogger logger(stdout, params.logLevel(), timeCache);
  ESB::Logger::SetInstance(&logger);

  error = ESB::SignalHandler::Instance().initialize();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot install signal handlers");
    return error;
  }

  //
  // Max out open files
  //

  error = ESB::SystemConfig::Instance().setSocketSoftMax(ESB::SystemConfig::Instance().socketHardMax());
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "[main] cannot raise max fd limit");
    return error;
  }

  //
  // Create listening socket
  //

  ESB::ListeningSocket listener("proxy-listener",
                                ESB::SocketAddress("0.0.0.0", params.proxyPort(),
                                                   params.secure() ? ESB::SocketAddress::TLS : ESB::SocketAddress::TCP),
                                ESB_UINT16_MAX);

  error = listener.bind();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "[main] cannot bind to port %u", listener.listeningAddress().port());
    return error;
  }

  ESB_LOG_NOTICE("[%s] bound to port %u", listener.name(), listener.listeningAddress().port());

  // Init

  ESB::SocketAddress originAddress(params.destinationAddress(), params.originPort(),
                                   params.secure() ? ESB::SocketAddress::TLS : ESB::SocketAddress::TCP);
  HttpFixedRouter router(originAddress);
  HttpRoutingProxyHandler handler(router);
  HttpProxy proxy("prox", params.proxyThreads(), params.proxyTimeoutMsec(), handler);

  error = proxy.initialize();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "[main] cannot initialize proxy");
    return error;
  }

  if (params.secure()) {
    ESB::TLSContext::Params tlsParams;
    error = proxy.serverTlsContextIndex().indexDefaultContext(tlsParams.privateKeyPath(params.serverKeyPath())
                                                                  .certificatePath(params.serverCertPath())
                                                                  .verifyPeerCertificate(false));
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot initialize proxy's default TLS server context");
      return error;
    }

    error = proxy.clientTlsContextIndex().indexDefaultContext(
        tlsParams.reset().caCertificatePath(params.caPath()).verifyPeerCertificate(true));
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot initialize proxy's default TLS client context");
      return error;
    }
  }

  // Start

  error = proxy.start();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "[main] cannot start proxy");
    return error;
  }

  // add listening sockets to running server

  error = proxy.addListener(listener);
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "[main] cannot add listener");
    return error;
  }

  // Wait for ctrl-C

  while (ESB::SignalHandler::Instance().running()) {
    usleep(100);
  }

  // Stop server

  proxy.stop();
  timeCache.stop();

  error = proxy.join();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "[main] cannot join proxy");
    return error;
  }

  proxy.serverCounters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);
  proxy.destroy();

  error = timeCache.join();
  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot join time cache thread");
    return error;
  }

  return 0;
}
