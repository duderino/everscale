#ifndef ESB_CONSOLE_LOGGER_H
#include <ESBConsoleLogger.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_EPOLL_MULTIPLEXER_FACTORY_H
#include <ESBEpollMultiplexerFactory.h>
#endif

#ifndef ESB_SOCKET_MULTIPLEXER_DISPATCHER_H
#include <ESBSocketMultiplexerDispatcher.h>
#endif

#ifndef EST_LISTENING_SOCKET_H
#include <ESTListeningSocket.h>
#endif

#ifndef ESB_FIXED_ALLOCATOR_H
#include <ESBFixedAllocator.h>
#endif

#ifndef ESB_SHARED_ALLOCATOR_H
#include <ESBSharedAllocator.h>
#endif

#ifndef EST_SERVER_SOCKET_H
#include <ESTServerSocket.h>
#endif

#ifndef ESB_ALLOCATOR_CLEANUP_HANDLER_H
#include <ESBAllocatorCleanupHandler.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void ESTServerSignalHandler(int signal);
static volatile ESB::Word IsRunning = 1;

static void printHelp() {
  fprintf(stderr, "Usage: -l <logLevel> -m <threads> -p <port>\n");
}

int main(int argc, char **argv) {
  ESB::UInt16 port = 8080;
  ESB::UInt16 multiplexerCount = 4;
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

  ESB::ConsoleLogger::Initialize((ESB::Logger::Severity)logLevel);
  ESB::Logger *logger = ESB::ConsoleLogger::Instance();

  if (logger->isLoggable(ESB::Logger::Notice)) {
    logger->log(ESB::Logger::Notice, __FILE__, __LINE__,
                "[main] starting. logLevel: %d, threads: %d, port: %d",
                logLevel, multiplexerCount, port);
  }

  //
  // Install signal handlers
  //

  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, ESTServerSignalHandler);
  signal(SIGQUIT, ESTServerSignalHandler);
  signal(SIGTERM, ESTServerSignalHandler);

  ESB::DiscardAllocator discardAllocator(4000,
                                         ESB::SystemAllocator::GetInstance());
  ESB::SharedAllocator rootAllocator(&discardAllocator);
  ESB::AllocatorCleanupHandler rootAllocatorCleanupHandler(&rootAllocator);

  ESB::Error error = rootAllocator.initialize();

  if (ESB_SUCCESS != error) {
    if (logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                  "[main] Cannot initialize root allocator: %s", buffer);
    }

    return error;
  }

  ESB::EpollMultiplexerFactory epollFactory("EpollMultiplexer", logger,
                                            &rootAllocator);

  ESB::UInt16 maxSockets =
      ESB::SocketMultiplexerDispatcher::GetMaximumSockets();

  if (logger->isLoggable(ESB::Logger::Notice)) {
    logger->log(ESB::Logger::Notice, __FILE__, __LINE__,
                "[main] Maximum sockets %d", maxSockets);
  }

  ESB::FixedAllocator fixedAllocator(maxSockets, sizeof(EST::ServerSocket),
                                     ESB::SystemAllocator::GetInstance());
  ESB::SharedAllocator socketAllocator(&fixedAllocator);
  ESB::AllocatorCleanupHandler socketAllocatorCleanupHandler(&socketAllocator);

  error = socketAllocator.initialize();

  if (ESB_SUCCESS != error) {
    if (logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                  "[main] Cannot initialize socket allocator: %s", buffer);
    }

    return error;
  }

  ESB::ListeningTCPSocket listeningSocket(port, ESB_UINT16_MAX, false);

  error = listeningSocket.bind();

  if (ESB_SUCCESS != error) {
    if (logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                  "[main] Cannot bind to port %d: %s", port, buffer);
    }

    return error;
  }

  error = listeningSocket.listen();

  if (ESB_SUCCESS != error) {
    if (logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                  "[main] Cannot listen on port %d: %s", port, buffer);
    }

    return error;
  }

  ESB::SocketMultiplexerDispatcher dispatcher(maxSockets, multiplexerCount,
                                              &epollFactory, &rootAllocator,
                                              "EpollDispatcher", logger);

  error = dispatcher.start();

  if (ESB_SUCCESS != error) {
    if (logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                  "[main] Cannot start multiplexer dispatcher: %s", buffer);
    }

    return error;
  }

  EST::ListeningSocket *socket = 0;

  for (int i = 0; i < multiplexerCount; ++i) {
    socket = new (&rootAllocator) EST::ListeningSocket(
        &listeningSocket, &socketAllocator, &dispatcher, logger,
        &rootAllocatorCleanupHandler, &socketAllocatorCleanupHandler);

    if (!socket) {
      if (logger->isLoggable(ESB::Logger::Critical)) {
        logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                    "[main] Cannot allocate new listening socket");
      }

      return ESB_OUT_OF_MEMORY;
    }

    error = dispatcher.addMultiplexedSocket(i, socket);

    if (ESB_SUCCESS != error) {
      if (logger->isLoggable(ESB::Logger::Critical)) {
        char buffer[100];

        ESB::DescribeError(error, buffer, sizeof(buffer));

        logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                    "[main] Cannot add listening socket to multiplexer: %s",
                    buffer);
      }

      return error;
    }
  }

  if (logger->isLoggable(ESB::Logger::Notice)) {
    logger->log(ESB::Logger::Notice, __FILE__, __LINE__, "[main] started");
  }

  while (IsRunning) {
    sleep(60);
  }

  if (logger->isLoggable(ESB::Logger::Notice)) {
    logger->log(ESB::Logger::Notice, __FILE__, __LINE__, "[main] stopping");
  }

  dispatcher.stop();

  if (logger->isLoggable(ESB::Logger::Notice)) {
    logger->log(ESB::Logger::Notice, __FILE__, __LINE__, "[main] stopped");
  }

  ESB::ConsoleLogger::Destroy();

  return ESB_SUCCESS;
}

void ESTServerSignalHandler(int signal) { IsRunning = 0; }
