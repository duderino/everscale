/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
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

#ifndef AWS_CLIENT_SOCKET_H
#include <AWSClientSocket.h>
#endif

#ifndef ESF_SHARED_ALLOCATOR_H
#include <ESFSharedAllocator.h>
#endif

#ifndef ESF_ALLOCATOR_CLEANUP_HANDLER_H
#include <ESFAllocatorCleanupHandler.h>
#endif

#ifndef AWS_PERFORMANCE_COUNTER_H
#include <AWSPerformanceCounter.h>
#endif

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static void AWSRawEchoClientSignalHandler(int signal);
static volatile ESFWord IsRunning = 1;
static AWSPerformanceCounter SuccessCounter("successes");
static struct timeval StopTime;

static void printHelp()
{
    fprintf(stderr, "Usage: -l <logLevel> -m <threads> -a <serverIp> -p <serverPort> -s <sockets> -r\n");
}

int main(int argc, char **argv)
{
    ESFUInt16 multiplexerCount = 4;
    const char *dottedIp = "127.0.0.1";
    ESFUInt16 port = 8080;
    ESFInt16 sockets = 1;
    bool reuseConnections = false;
    int logLevel = ESFLogger::Notice;

    {
        int result = 0;

        while (true)
        {
            result = getopt(argc, argv, "l:m:a:p:s:r");

            if (0 > result)
            {
                break;
            }

            switch (result)
            {
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

                case 'a':

                    dottedIp = optarg;
                    break;

                case 'p':

                    port = atoi(optarg);
                    break;

                case 's':

                    sockets = atoi(optarg);
                    break;

                case 'r':

                    reuseConnections = true;
                    break;

                default:

                    printHelp();

                    return 2;
            }
        }
    }

    ESFConsoleLogger::Initialize((ESFLogger::Severity) logLevel);
    ESFLogger *logger = ESFConsoleLogger::Instance();

    if (logger->isLoggable(ESFLogger::Notice))
    {
        logger->log(ESFLogger::Notice, __FILE__, __LINE__,
                    "[main] starting. logLevel: %d, threads: %d, server: %s, port: %d, sockets: %d, reuseConnections: %s",
                    logLevel, multiplexerCount, dottedIp, port, sockets,  reuseConnections ? "true" : "false");
    }

    AWSClientSocket::SetReuseConnections(reuseConnections);
    ESFSocketAddress serverAddress( dottedIp, port, ESFSocketAddress::TCP );

    //
    // Install signal handlers
    //

    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, AWSRawEchoClientSignalHandler);
    signal(SIGQUIT, AWSRawEchoClientSignalHandler);
    signal(SIGTERM, AWSRawEchoClientSignalHandler);



    ESFDiscardAllocator discardAllocator(4000, ESFSystemAllocator::GetInstance());
    ESFSharedAllocator rootAllocator(&discardAllocator);
    ESFAllocatorCleanupHandler rootAllocatorCleanupHandler(&rootAllocator);

    ESFError error = rootAllocator.initialize();

    if (ESF_SUCCESS != error)
    {
        if (logger->isLoggable(ESFLogger::Critical))
        {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                        "[main] Cannot initialize root allocator: %s", buffer);
        }

        return error;
    }

    ESFEpollMultiplexerFactory epollFactory("EpollMultiplexer",
                                            logger,
                                            &rootAllocator);

    ESFUInt16 maxSockets = ESFSocketMultiplexerDispatcher::GetMaximumSockets();

    if (sockets > maxSockets)
    {
        if (logger->isLoggable(ESFLogger::Critical))
        {
            logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                        "Raise ulimit -n, only %d sockets can be created\n", maxSockets);
        }

        return 1;
    }

    if (logger->isLoggable(ESFLogger::Notice))
    {
        logger->log(ESFLogger::Notice, __FILE__, __LINE__,
                    "[main] Maximum sockets %d", maxSockets);
    }

    ESFSocketMultiplexerDispatcher dispatcher(maxSockets,
                                              multiplexerCount,
                                              &epollFactory,
                                              &rootAllocator,
                                              "EpollDispatcher",
                                              logger);


    error = dispatcher.start();

    if (ESF_SUCCESS != error)
    {
        if (logger->isLoggable(ESFLogger::Critical))
        {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                        "[main] Cannot start multiplexer dispatcher: %s", buffer);
        }

        return error;
    }

    AWSClientSocketFactory factory(maxSockets,
                                   &SuccessCounter,
                                   &dispatcher,
                                   logger);

    sleep(1);   // give the worker threads a chance to start - cleans up perf testing numbers a bit

    struct timeval startTime;
    AWSPerformanceCounter::GetTime(&startTime);

    for (int i = 0; i < sockets; ++i)
    {
        error = factory.addNewConnection(serverAddress);

        if (ESF_SUCCESS != error)
        {
            if (logger->isLoggable(ESFLogger::Critical))
            {
                char buffer[100];

                ESFDescribeError(error, buffer, sizeof(buffer));

                logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                            "[main] Cannot add new connection: %s", buffer);
            }

            return error;
        }
    }

    if (logger->isLoggable(ESFLogger::Notice))
    {
        logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[main] started");
    }

    while (IsRunning)
    {
        sleep(60);
    }

    if (logger->isLoggable(ESFLogger::Notice))
    {
        logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[main] stopping");
    }

    dispatcher.stop();

    if (logger->isLoggable(ESFLogger::Notice))
    {
        logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[main] stopped");
    }

    ESFConsoleLogger::Destroy();

    ESFUInt32 seconds = StopTime.tv_sec - startTime.tv_sec;
    ESFUInt32 microseconds = 0;

    if (StopTime.tv_usec < startTime.tv_usec)
    {
        --seconds;
        microseconds = StopTime.tv_usec - (ESF_UINT32_C(1000000) - startTime.tv_usec);
    }
    else
    {
        microseconds = StopTime.tv_usec - startTime.tv_usec;
    }

    SuccessCounter.printSummary();
    fprintf(stdout, "\tTotal Sec: %u.%u\n", seconds, microseconds);

    return ESF_SUCCESS;
}

void AWSRawEchoClientSignalHandler(int signal)
{
    IsRunning = 0;
    AWSPerformanceCounter::GetTime(&StopTime);
}

