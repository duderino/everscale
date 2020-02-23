/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef ESF_CONSOLE_LOGGER_H
#include <ESFConsoleLogger.h>
#endif

#ifndef AWS_HTTP_STACK_H
#include <AWSHttpStack.h>
#endif

#ifndef AWS_HTTP_ECHO_SERVER_HANDLER_H
#include <AWSHttpEchoServerHandler.h>
#endif

#ifndef AWS_HTTP_DEFAULT_RESOLVER_H
#include <AWSHttpDefaultResolver.h>
#endif

#ifndef AWS_HTTP_CLIENT_SIMPLE_COUNTERS_H
#include <AWSHttpClientSimpleCounters.h>
#endif

#ifndef AWS_HTTP_SERVER_SIMPLE_COUNTERS_H
#include <AWSHttpServerSimpleCounters.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void AWSHttpEchoServerSignalHandler(int signal);
static volatile ESFWord IsRunning = 1;

static void printHelp() {
  fprintf(stderr, "Usage: -l <logLevel> -m <threads> -p <port>\n");
}

int main(int argc, char **argv) {
  int port = 8080;
  int threads = 4;
  int logLevel = ESFLogger::Debug;

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

  ESFConsoleLogger::Initialize((ESFLogger::Severity)logLevel);
  ESFLogger *logger = ESFConsoleLogger::Instance();

  if (logger->isLoggable(ESFLogger::Notice)) {
    logger->log(ESFLogger::Notice, __FILE__, __LINE__,
                "[main] starting. logLevel: %d, threads: %d, port: %d",
                logLevel, threads, port);
  }

  //
  // Install signal handlers
  //

  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, AWSHttpEchoServerSignalHandler);
  signal(SIGQUIT, AWSHttpEchoServerSignalHandler);
  signal(SIGTERM, AWSHttpEchoServerSignalHandler);

  AWSHttpEchoServerHandler handler(logger);
  AWSHttpDefaultResolver resolver(logger);
  AWSHttpClientSimpleCounters clientCounters;
  AWSHttpServerSimpleCounters serverCounters;

  AWSHttpStack stack(&handler, &resolver, port, threads, &clientCounters,
                     &serverCounters, logger);

  ESFError error = stack.initialize();

  if (ESF_SUCCESS != error) {
    return -1;
  }

  error = stack.start();

  if (ESF_SUCCESS != error) {
    return -2;
  }

  while (IsRunning) {
    sleep(60);
  }

  error = stack.stop();

  if (ESF_SUCCESS != error) {
    return -3;
  }

  serverCounters.printSummary(stdout);

  stack.destroy();

  ESFConsoleLogger::Destroy();

  return 0;
}

void AWSHttpEchoServerSignalHandler(int signal) { IsRunning = 0; }
