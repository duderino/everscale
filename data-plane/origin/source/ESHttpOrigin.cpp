#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
#endif

#ifndef ES_HTTP_TEST_PARAMS_H
#include <ESHttpTestParams.h>
#endif

#ifndef ES_EPHEMERAL_LISTENER_H
#include <ESEphemeralListener.h>
#endif

#ifndef ES_HTTP_ORIGIN_HANDLER_H
#include <ESHttpOriginHandler.h>
#endif

#ifndef ES_HTTP_SERVER_H
#include <ESHttpServer.h>
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef ESB_SIGNAL_HANDLER_H
#include <ESBSignalHandler.h>
#endif

using namespace ES;

int main(int argc, char **argv) {
  HttpTestParams params;
  params.originThreads(3).port(8080).logLevel(ESB::Logger::Notice);
  ESB::Error error = params.override(argc, argv);
  if (ESB_SUCCESS != error) {
    return error;
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

  ESB::ListeningSocket listener("origin-listener",ESB::SocketAddress("0.0.0.0", params.port(), params.secure() ? ESB::SocketAddress::TLS : ESB::SocketAddress::TCP), ESB_UINT16_MAX);

  error = listener.bind();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "[main] cannot bind to port %u", listener.listeningAddress().port());
    return error;
  }

  ESB_LOG_NOTICE("[%s] bound to port %u", listener.name(), listener.listeningAddress().port());

  // Init

  HttpOriginHandler handler(params);
  HttpServer server("origin", params.originThreads(), params.originTimeoutMsec(), handler);

  error = server.initialize();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "[main] cannot initialize server");
    return error;
  }

  if (params.secure()) {
    ESB::TLSContext::Params tlsParams;
    error = server.serverTlsContextIndex().indexDefaultContext(tlsParams.privateKeyPath(params.serverKeyPath())
                                                                               .certificatePath(params.serverCertPath())
                                                                               .verifyPeerCertificate(false));
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot initialize server's default TLS server context");
      return error;
    }
  }

  // Start

  error = server.start();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "[main] cannot start server");
    return error;
  }

  // add listening sockets to running server

  error = server.addListener(listener);
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "[main] cannot add listener");
    return error;
  }

  // Wait for ctrl-C

  while (ESB::SignalHandler::Instance().running()) {
    usleep(100);
  }

  // Stop server

  server.stop();
  timeCache.stop();

  error = server.join();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "[main] cannot join server");
    return error;
  }

  server.serverCounters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);
  server.destroy();

  error = timeCache.join();
  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot join time cache thread");
    return error;
  }

  return 0;
}
