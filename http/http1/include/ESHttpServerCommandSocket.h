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
  HttpServerCommandSocket(HttpMultiplexerExtended &stack);

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

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 protected:
  virtual ESB::Error runCommand(ESB::EmbeddedListElement *command);

 private:
  // Disabled
  HttpServerCommandSocket(const HttpServerCommandSocket &);
  HttpServerCommandSocket &operator=(const HttpServerCommandSocket &);

  HttpMultiplexerExtended &_multiplexer;
};

}  // namespace ES

#endif
