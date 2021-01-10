#ifndef ES_HTTP_LOADGEN_CONTEXT_H
#include <ESHttpLoadgenContext.h>
#endif

#ifndef ES_HTTP_LOADGEN_HANDLER_H
#include <ESHttpLoadgenHandler.h>
#endif

#ifndef ES_HTTP_LOADGEN_SEED_COMMAND_H
#include <ESHttpLoadgenSeedCommand.h>
#endif

#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ES_HTTP_CLIENT_H
#include <ESHttpClient.h>
#endif

#ifndef ES_HTTP_CLIENT_SOCKET_H
#include <ESHttpClientSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
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
  params.connections(1).requestsPerConnection(3).clientThreads(3).logLevel(ESB::Logger::Notice);
  ESB::Error error = params.override(argc, argv);
  if (ESB_SUCCESS != error) {
    return error;
  }

  HttpLoadgenContext::SetTotalIterations(params.connections() * params.requestsPerConnection());

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
    ESB_LOG_ERROR_ERRNO(error, "[main] cannot raise max fd limit");
    return -5;
  }

  //
  // Create, initialize, and start the stack
  //

  HttpLoadgenHandler clientHandler(params);
  HttpClientSocket::SetReuseConnections(params.reuseConnections());
  HttpClient client("loadgen", params.clientThreads(), clientHandler);
  ESB::SocketAddress destination(params.destinationAddress(), params.port(), ESB::SocketAddress::TransportType::TCP);

  error = client.initialize();
  if (ESB_SUCCESS != error) {
    return error;
  }

  error = client.start();
  if (ESB_SUCCESS != error) {
    return error;
  }

  for (int i = 0; i < client.threads(); ++i) {
    HttpLoadgenSeedCommand *command = new (ESB::SystemAllocator::Instance())
        HttpLoadgenSeedCommand(destination, params, ESB::SystemAllocator::Instance().cleanupHandler());
    error = client.push(command, i);
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "[main] cannot push seed command");
      return error;
    }
  }

  while (IsRunning && !HttpLoadgenContext::IsFinished()) {
    sleep(1);
  }

  error = client.stop();
  if (ESB_SUCCESS != error) {
    return error;
  }

  client.clientCounters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);
  client.destroy();

  return ESB_SUCCESS;
}
