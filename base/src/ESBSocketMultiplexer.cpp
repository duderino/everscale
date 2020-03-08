#ifndef ESB_SOCKET_MULTIPLEXER_H
#include <ESBSocketMultiplexer.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

namespace ESB {

SocketMultiplexer::SocketMultiplexer(const char *name)
    : _name(name ? name : "SocketMultiplexer") {}

SocketMultiplexer::~SocketMultiplexer() {}

}  // namespace ESB
