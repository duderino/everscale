#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
#endif

#ifndef ES_HTTP_SERVER_H
#include <ESHttpServer.h>
#endif

#ifndef ES_HTTP_ECHO_SERVER_HANDLER_H
#include <ESHttpEchoServerHandler.h>
#endif

#ifndef ESB_SYSTEM_DNS_CLIENT_H
#include <ESBSystemDnsClient.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

using namespace ES;

static void HttpEchoServerSignalHandler(int signal);
static volatile ESB::Word IsRunning = 1;

static void printHelp() {
  fprintf(stderr, "Usage: -l <logLevel> -m <threads> -p <port>\n");
}

int main(int argc, char **argv) {
  int port = 8080;
  int threads = 4;
  int logLevel = ESB::Logger::Notice;

  {
    int result = 0;

    while (true) {
      result = getopt(argc, argv, "l:m:p:");

      if (0 > result) {
        break;
      }

      switch (result) {
        case 'l':

          /*
          None = 0,
          Emergency = 1,   System-wide non-recoverable error.
          Alert = 2,       System-wide non-recoverable error imminent.
          Critical = 3,    System-wide potentially recoverable error.
          Error = 4,       Localized non-recoverable error.
          Warning = 5,     Localized potentially recoverable error.
          Notice = 6,      Important non-error event.
          Info = 7,        Non-error event.
          Debug = 8        Debugging event.
          */

          logLevel = atoi(optarg);
          break;

        case 'm':

          threads = atoi(optarg);
          break;

        case 'p':

          port = atoi(optarg);
          break;

        default:

          printHelp();

          return 2;
      }
    }
  }

  ESB::SimpleFileLogger logger;
  logger.setSeverity((ESB::Logger::Severity)logLevel);
  ESB::Logger::SetInstance(&logger);

  ESB_LOG_NOTICE("[main] starting. logLevel: %d, threads: %d, port: %d",
                 logLevel, threads, port);

  //
  // Install signal handlers
  //

  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, HttpEchoServerSignalHandler);
  signal(SIGQUIT, HttpEchoServerSignalHandler);
  signal(SIGTERM, HttpEchoServerSignalHandler);

  //
  // Max out open files
  //

  ESB::Error error = ESB::SystemConfig::Instance().setSocketSoftMax(
      ESB::SystemConfig::Instance().socketHardMax());

  ESB_LOG_ERROR_ERRNO(error, "Cannot raise max fd limit");

  HttpEchoServerHandler handler;
  HttpServer server(threads, port, handler);

  error = server.initialize();

  if (ESB_SUCCESS != error) {
    return -1;
  }

  error = server.start();

  if (ESB_SUCCESS != error) {
    return -2;
  }

  while (IsRunning) {
    sleep(1);
  }

  error = server.stop();

  if (ESB_SUCCESS != error) {
    return -3;
  }

  server.counters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);
  server.destroy();

  return 0;
}

void HttpEchoServerSignalHandler(int signal) { IsRunning = 0; }
