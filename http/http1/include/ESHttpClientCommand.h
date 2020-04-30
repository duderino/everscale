#ifndef ES_HTTP_CLIENT_COMMAND_H
#define ES_HTTP_CLIENT_COMMAND_H

#ifndef ES_HTTP_MULTIPLEXER_H
#include <ESHttpMultiplexer.h>
#endif

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

namespace ES {

class HttpClientCommand : public ESB::EmbeddedListElement {
 public:
  HttpClientCommand();

  virtual ~HttpClientCommand();

  /**
   * Execute on a multiplexer thread.
   *
   * @param multiplexer The API for the multiplexer thread.
   * @return ESB_SUCCESS if successful, another error code otherwise
   */
  virtual ESB::Error run(HttpMultiplexer &multiplexer) = 0;

  virtual const char *name() = 0;

 private:
  // Disabled
  HttpClientCommand(const HttpClientCommand &serverCommand);
  void operator=(const HttpClientCommand &serverCommand);
};

}  // namespace ES

#endif
