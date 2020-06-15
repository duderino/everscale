#ifndef ES_HTTP_SERVER_COMMAND_SOCKET_H
#include <ESHttpServerCommandSocket.h>
#endif

namespace ES {

HttpServerCommandSocket::HttpServerCommandSocket(const char *namePrefix, HttpMultiplexerExtended &stack)
    : HttpCommandSocket(namePrefix), _multiplexer(stack) {}

HttpServerCommandSocket::~HttpServerCommandSocket() {}

// This code runs in any thread
ESB::Error HttpServerCommandSocket::push(HttpServerCommand *command) { return pushInternal(command); }

ESB::Error HttpServerCommandSocket::runCommand(ESB::EmbeddedListElement *element) {
  HttpServerCommand *command = (HttpServerCommand *)element;

  ESB_LOG_DEBUG("[%s] executing command '%s'", name(), ESB_SAFE_STR(command->name()));
  ESB::Error error = command->run(_multiplexer);

  if (ESB_SUCCESS != error) {
    ESB_LOG_WARNING_ERRNO(error, "[%s] cannot execute command '%s", name(), ESB_SAFE_STR(command->name()));
  }

  return error;
}

}  // namespace ES
