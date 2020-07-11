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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

static volatile ESB::Word IsRunning = 1;
static void SignalHandler(int signal) { IsRunning = 0; }

using namespace ES;

int main(int argc, char **argv) {
  HttpTestParams params;
  params.originThreads(3).port(8080).logLevel(ESB::Logger::Notice);
  ESB::Error error = params.override(argc, argv);
  if (ESB_SUCCESS != error) {
    return error;
  }

  ESB::Time::Instance().start();
  ESB::SimpleFileLogger logger(stdout);
  logger.setSeverity(params.logLevel());
  ESB::Logger::SetInstance(&logger);

  //
  // Install signal handlers: Ctrl-C and kill will start clean shutdown sequence
  //

  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, SignalHandler);
  signal(SIGQUIT, SignalHandler);
  signal(SIGTERM, SignalHandler);

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

  ESB::ListeningTCPSocket listener("origin-listener", params.port(), ESB_UINT16_MAX);

  error = listener.bind();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "[main] cannot bind to port %u", listener.listeningAddress().port());
    return error;
  }

  ESB_LOG_NOTICE("[%s] bound to port %u", listener.name(), listener.listeningAddress().port());

  error = listener.listen();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "[main] cannot listen on port %u", listener.listeningAddress().port());
    return error;
  }

  // Init

  HttpOriginHandler handler(params);
  HttpServer server("origin", params.originThreads(), handler);

  error = server.initialize();
  if (ESB_SUCCESS != error) {
    return error;
  }

  // Start

  error = server.start();
  if (ESB_SUCCESS != error) {
    return error;
  }

  // add listening sockets to running server

  error = server.addListener(listener);
  if (ESB_SUCCESS != error) {
    return error;
  }

  // Wait for ctrl-C

  while (IsRunning) {
    sleep(1);
  }

  // Stop server

  error = server.stop();

  if (ESB_SUCCESS != error) {
    return error;
  }

  server.serverCounters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);
  server.destroy();

  return 0;
}

void HttpOriginSignalHandler(int signal) { IsRunning = 0; }
