#ifndef ESB_SOCKET_MULTIPLEXER_H
#include <ESBSocketMultiplexer.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

namespace ESB {

SocketMultiplexer::SocketMultiplexer(const char *name, Logger *logger)
    : _name(name ? name : "SocketMultiplexer"),
      _logger(logger ? logger : NullLogger::GetInstance()) {}

SocketMultiplexer::~SocketMultiplexer() {}

}  // namespace ESB
