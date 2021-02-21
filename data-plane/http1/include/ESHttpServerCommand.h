#ifndef ES_HTTP_SERVER_COMMAND_H
#define ES_HTTP_SERVER_COMMAND_H

#ifndef ES_HTTP_MULTIPLEXER_EXTENDED_H
#include <ESHttpMultiplexerExtended.h>
#endif

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

namespace ES {

class HttpServerCommand : public ESB::EmbeddedListElement {
 public:
  HttpServerCommand();

  virtual ~HttpServerCommand();

  /**
   * Execute on a multiplexer thread.
   *
   * @param multiplexer The API for the multiplexer thread.
   * @return ESB_SUCCESS if successful, another error code otherwise
   */
  virtual ESB::Error run(HttpMultiplexerExtended &multiplexer) = 0;

  virtual const char *name() = 0;

 private:
  // Disabled
  HttpServerCommand(const HttpServerCommand &serverCommand);
  void operator=(const HttpServerCommand &serverCommand);
};

}  // namespace ES

#endif
