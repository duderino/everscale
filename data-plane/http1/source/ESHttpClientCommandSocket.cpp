#ifndef ES_HTTP_CLIENT_COMMAND_SOCKET_H
#include <ESHttpClientCommandSocket.h>
#endif

namespace ES {

HttpClientCommandSocket::HttpClientCommandSocket(const char *namePrefix, HttpMultiplexerExtended &stack)
    : HttpCommandSocket(namePrefix), _stack(stack) {}

HttpClientCommandSocket::~HttpClientCommandSocket() {}

// This code runs in any thread
ESB::Error HttpClientCommandSocket::push(HttpClientCommand *command) { return pushInternal(command); }

ESB::Error HttpClientCommandSocket::runCommand(ESB::EmbeddedListElement *element) {
  HttpClientCommand *command = (HttpClientCommand *)element;

  ESB_LOG_DEBUG("[%s] executing command '%s'", name(), ESB_SAFE_STR(command->name()));
  ESB::Error error = command->run(_stack);

  if (ESB_SUCCESS != error) {
    ESB_LOG_WARNING_ERRNO(error, "[%s] cannot execute command '%s'", name(), ESB_SAFE_STR(command->name()));
  }

  return error;
}

}  // namespace ES
