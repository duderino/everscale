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

#ifndef ESB_TIME_SOURCE_CACHE_H
#include <ESBTimeSourceCache.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
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
  params.connections(1).requestsPerConnection(3).clientThreads(3).logLevel(ESB::Logger::Notice);
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

  HttpLoadgenContext::SetTotalIterations(params.connections() * params.requestsPerConnection());

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
  HttpClient client("loadgen", params.clientThreads(), params.clientTimeoutMsec(), clientHandler);
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

  while (ESB::SignalHandler::Instance().running() && !HttpLoadgenContext::IsFinished()) {
    usleep(100);
  }

  client.stop();
  timeCache.stop();

  error = client.join();
  if (ESB_SUCCESS != error) {
    return error;
  }

  client.clientCounters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);
  client.destroy();

  error = timeCache.join();
  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot join time cache thread");
    return error;
  }

  return ESB_SUCCESS;
}
