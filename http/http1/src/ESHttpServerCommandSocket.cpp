#ifndef ES_HTTP_SERVER_COMMAND_SOCKET_H
#include <ESHttpServerCommandSocket.h>
#endif

namespace ES {

HttpServerCommandSocket::HttpServerCommandSocket(HttpServerStack &stack)
    : _stack(stack) {}

HttpServerCommandSocket::~HttpServerCommandSocket() {}

// This code runs in any thread
ESB::Error HttpServerCommandSocket::push(HttpServerCommand *command) {
  return pushInternal(command);
}

ESB::Error HttpServerCommandSocket::runCommand(
    ESB::EmbeddedListElement *element) {
  HttpServerCommand *command = (HttpServerCommand *)element;

  ESB_LOG_DEBUG("Executing command '%s' in multiplexer",
                ESB_SAFE_STR(command->name()));
  ESB::Error error = command->run(_stack);

  if (ESB_SUCCESS != error) {
    ESB_LOG_WARNING_ERRNO(error, "Failed executing command '%s in multiplexer",
                          ESB_SAFE_STR(command->name()));
  }

  return error;
}

}  // namespace ES
