#ifndef ESB_CONSOLE_LOGGER_H
#include <ESBConsoleLogger.h>
#endif

#ifndef ES_HTTP_STACK_H
#include <ESHttpStack.h>
#endif

#ifndef ES_HTTP_ECHO_SERVER_HANDLER_H
#include <ESHttpEchoServerHandler.h>
#endif

#ifndef ESB_SYSTEM_DNS_CLIENT_H
#include <ESBSystemDnsClient.h>
#endif

#ifndef ESB_PROCESS_LIMITS_H
#include <ESBProcessLimits.h>
#endif

#ifndef ES_HTTP_CLIENT_SIMPLE_COUNTERS_H
#include <ESHttpClientSimpleCounters.h>
#endif

#ifndef ES_HTTP_SERVER_SIMPLE_COUNTERS_H
#include <ESHttpServerSimpleCounters.h>
#endif

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

using namespace ES;

static void HttpEchoServerSignalHandler(int signal);
static volatile ESB::Word IsRunning = 1;

static void printHelp() {
  fprintf(stderr, "Usage: -l <logLevel> -m <threads> -p <port>\n");
}

int main(int argc, char **argv) {
  int port = 8080;
  int threads = 4;
  int logLevel = ESB::Logger::Debug;

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

  ESB::ConsoleLogger::Initialize((ESB::Logger::Severity)logLevel);
  ESB::Logger *logger = ESB::ConsoleLogger::Instance();

  if (logger->isLoggable(ESB::Logger::Notice)) {
    logger->log(ESB::Logger::Notice, __FILE__, __LINE__,
                "[main] starting. logLevel: %d, threads: %d, port: %d",
                logLevel, threads, port);
  }

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

  ESB::Error error = ESB::ProcessLimits::SetSocketSoftMax(ESB::ProcessLimits::GetSocketHardMax());

  if (ESB_SUCCESS != error) {
    if (logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[256];
      ESB::DescribeError(error, buffer, sizeof(buffer));
      logger->log(ESB::Logger::Critical, __FILE__, __LINE__,"Cannot raise max fd limit: %s", buffer);
    }
    return -5;
  }

  HttpEchoServerHandler handler(logger);
  ESB::SystemDnsClient dnsClient(logger);
  HttpClientSimpleCounters clientCounters;
  HttpServerSimpleCounters serverCounters;

  HttpStack stack(&handler, &dnsClient, port, threads, &clientCounters,
                  &serverCounters, logger);

  error = stack.initialize();

  if (ESB_SUCCESS != error) {
    return -1;
  }

  error = stack.start();

  if (ESB_SUCCESS != error) {
    return -2;
  }

  while (IsRunning) {
    sleep(1);
  }

  error = stack.stop();

  if (ESB_SUCCESS != error) {
    return -3;
  }

  serverCounters.printSummary(stdout);

  stack.destroy();

  ESB::ConsoleLogger::Destroy();

  return 0;
}

void HttpEchoServerSignalHandler(int signal) { IsRunning = 0; }
