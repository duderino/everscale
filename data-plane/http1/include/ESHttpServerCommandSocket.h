#ifndef ES_HTTP_SERVER_COMMAND_SOCKET_H
#define ES_HTTP_SERVER_COMMAND_SOCKET_H

#ifndef ES_HTTP_MULTIPLEXER_EXTENDED_H
#include <ESHttpMultiplexerExtended.h>
#endif

#ifndef ES_HTTP_SERVER_COMMAND_H
#include <ESHttpServerCommand.h>
#endif

#ifndef ES_HTTP_COMMAND_SOCKET_H
#include <ESHttpCommandSocket.h>
#endif

namespace ES {

/** A socket that can wake up multiplexers to run HttpServerCommands.
 */
class HttpServerCommandSocket : public HttpCommandSocket {
 public:
  /** Constructor
   */
  HttpServerCommandSocket(const char *namePrefix, HttpMultiplexerExtended &stack);

  /** Destructor.
   */
  virtual ~HttpServerCommandSocket();

  /**
   * Enqueue a command on the command socket.  When the command socket is
   * in a multiplexer, the multiplexer will wake up, dequeue the command,
   * and execute it on the multiplexer's thread of control.
   *
   * @param command The command to execute
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error push(HttpServerCommand *command);

 protected:
  virtual ESB::Error runCommand(ESB::EmbeddedListElement *command);

 private:
  HttpMultiplexerExtended &_multiplexer;

  ESB_DEFAULT_FUNCS(HttpServerCommandSocket);
};

}  // namespace ES

#endif
