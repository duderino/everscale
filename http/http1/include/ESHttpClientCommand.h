#ifndef ES_HTTP_CLIENT_COMMAND_H
#define ES_HTTP_CLIENT_COMMAND_H

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

#ifndef ES_HTTP_CLIENT_STACK_H
#include <ESHttpClientStack.h>
#endif

namespace ES {

class HttpClientCommand : public ESB::EmbeddedListElement {
 public:
  HttpClientCommand();

  virtual ~HttpClientCommand();

  /**
   * Execute on a multiplexer thread.
   *
   * @param stack The stack for the multiplexer thread.
   * @return ESB_SUCCESS if successful, another error code otherwise
   */
  virtual ESB::Error run(HttpClientStack &stack) = 0;

  virtual const char *name() = 0;

 private:
  // Disabled
  HttpClientCommand(const HttpClientCommand &serverCommand);
  void operator=(const HttpClientCommand &serverCommand);
};

}  // namespace ES

#endif
