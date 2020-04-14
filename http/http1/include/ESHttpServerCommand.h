#ifndef ES_HTTP_SERVER_COMMAND_H
#define ES_HTTP_SERVER_COMMAND_H

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

#ifndef ES_HTTP_SERVER_STACK_H
#include <ESHttpServerStack.h>
#endif

namespace ES {

class HttpServerCommand : public ESB::EmbeddedListElement {
 public:
  HttpServerCommand();

  virtual ~HttpServerCommand();

  /**
   * Execute on a multiplexer thread.
   *
   * @param stack The stack for the multiplexer thread.
   * @return ESB_SUCCESS if successful, another error code otherwise
   */
  virtual ESB::Error run(HttpServerStack &stack) = 0;

 private:
  // Disabled
  HttpServerCommand(const HttpServerCommand &serverCommand);
  void operator=(const HttpServerCommand &serverCommand);
};

}  // namespace ES

#endif
