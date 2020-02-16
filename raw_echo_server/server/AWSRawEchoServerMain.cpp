/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef ESF_CONSOLE_LOGGER_H
#include <ESFConsoleLogger.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_DISCARD_ALLOCATOR_H
#include <ESFDiscardAllocator.h>
#endif

#ifndef ESF_EPOLL_MULTIPLEXER_FACTORY_H
#include <ESFEpollMultiplexerFactory.h>
#endif

#ifndef ESF_SOCKET_MULTIPLEXER_DISPATCHER_H
#include <ESFSocketMultiplexerDispatcher.h>
#endif

#ifndef AWS_LISTENING_SOCKET_H
#include <AWSListeningSocket.h>
#endif

#ifndef ESF_FIXED_ALLOCATOR_H
#include <ESFFixedAllocator.h>
#endif

#ifndef ESF_SHARED_ALLOCATOR_H
#include <ESFSharedAllocator.h>
#endif

#ifndef AWS_SERVER_SOCKET_H
#include <AWSServerSocket.h>
#endif

#ifndef ESF_ALLOCATOR_CLEANUP_HANDLER_H
#include <ESFAllocatorCleanupHandler.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void AWSRawEchoServerSignalHandler(int signal);
static volatile ESFWord IsRunning = 1;

static void printHelp() {
  fprintf(stderr, "Usage: -l <logLevel> -m <threads> -p <port>\n");
}

int main(int argc, char **argv) {
  ESFUInt16 port = 8080;
  ESFUInt16 multiplexerCount = 4;
  int logLevel = ESFLogger::Notice;

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

          multiplexerCount = atoi(optarg);
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
                logLevel, multiplexerCount, port);
  }

  //
  // Install signal handlers
  //

  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, AWSRawEchoServerSignalHandler);
  signal(SIGQUIT, AWSRawEchoServerSignalHandler);
  signal(SIGTERM, AWSRawEchoServerSignalHandler);

  ESFDiscardAllocator discardAllocator(4000, ESFSystemAllocator::GetInstance());
  ESFSharedAllocator rootAllocator(&discardAllocator);
  ESFAllocatorCleanupHandler rootAllocatorCleanupHandler(&rootAllocator);

  ESFError error = rootAllocator.initialize();

  if (ESF_SUCCESS != error) {
    if (logger->isLoggable(ESFLogger::Critical)) {
      char buffer[100];

      ESFDescribeError(error, buffer, sizeof(buffer));

      logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                  "[main] Cannot initialize root allocator: %s", buffer);
    }

    return error;
  }

  ESFEpollMultiplexerFactory epollFactory("EpollMultiplexer", logger,
                                          &rootAllocator);

  ESFUInt16 maxSockets = ESFSocketMultiplexerDispatcher::GetMaximumSockets();

  if (logger->isLoggable(ESFLogger::Notice)) {
    logger->log(ESFLogger::Notice, __FILE__, __LINE__,
                "[main] Maximum sockets %d", maxSockets);
  }

  ESFFixedAllocator fixedAllocator(maxSockets, sizeof(AWSServerSocket),
                                   ESFSystemAllocator::GetInstance());
  ESFSharedAllocator socketAllocator(&fixedAllocator);
  ESFAllocatorCleanupHandler socketAllocatorCleanupHandler(&socketAllocator);

  error = socketAllocator.initialize();

  if (ESF_SUCCESS != error) {
    if (logger->isLoggable(ESFLogger::Critical)) {
      char buffer[100];

      ESFDescribeError(error, buffer, sizeof(buffer));

      logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                  "[main] Cannot initialize socket allocator: %s", buffer);
    }

    return error;
  }

  ESFListeningTCPSocket listeningSocket(port, ESF_UINT16_MAX, false);

  error = listeningSocket.bind();

  if (ESF_SUCCESS != error) {
    if (logger->isLoggable(ESFLogger::Critical)) {
      char buffer[100];

      ESFDescribeError(error, buffer, sizeof(buffer));

      logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                  "[main] Cannot bind to port %d: %s", port, buffer);
    }

    return error;
  }

  error = listeningSocket.listen();

  if (ESF_SUCCESS != error) {
    if (logger->isLoggable(ESFLogger::Critical)) {
      char buffer[100];

      ESFDescribeError(error, buffer, sizeof(buffer));

      logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                  "[main] Cannot listen on port %d: %s", port, buffer);
    }

    return error;
  }

  ESFSocketMultiplexerDispatcher dispatcher(maxSockets, multiplexerCount,
                                            &epollFactory, &rootAllocator,
                                            "EpollDispatcher", logger);

  error = dispatcher.start();

  if (ESF_SUCCESS != error) {
    if (logger->isLoggable(ESFLogger::Critical)) {
      char buffer[100];

      ESFDescribeError(error, buffer, sizeof(buffer));

      logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                  "[main] Cannot start multiplexer dispatcher: %s", buffer);
    }

    return error;
  }

  AWSListeningSocket *socket = 0;

  for (int i = 0; i < multiplexerCount; ++i) {
    socket = new (&rootAllocator) AWSListeningSocket(
        &listeningSocket, &socketAllocator, &dispatcher, logger,
        &rootAllocatorCleanupHandler, &socketAllocatorCleanupHandler);

    if (!socket) {
      if (logger->isLoggable(ESFLogger::Critical)) {
        logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                    "[main] Cannot allocate new listening socket");
      }

      return ESF_OUT_OF_MEMORY;
    }

    error = dispatcher.addMultiplexedSocket(i, socket);

    if (ESF_SUCCESS != error) {
      if (logger->isLoggable(ESFLogger::Critical)) {
        char buffer[100];

        ESFDescribeError(error, buffer, sizeof(buffer));

        logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                    "[main] Cannot add listening socket to multiplexer: %s",
                    buffer);
      }

      return error;
    }
  }

  if (logger->isLoggable(ESFLogger::Notice)) {
    logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[main] started");
  }

  while (IsRunning) {
    sleep(60);
  }

  if (logger->isLoggable(ESFLogger::Notice)) {
    logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[main] stopping");
  }

  dispatcher.stop();

  if (logger->isLoggable(ESFLogger::Notice)) {
    logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[main] stopped");
  }

  ESFConsoleLogger::Destroy();

  return ESF_SUCCESS;
}

void AWSRawEchoServerSignalHandler(int signal) { IsRunning = 0; }
