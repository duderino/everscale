#ifndef ES_HTTP_CLIENT_COMMAND_H
#define ES_HTTP_CLIENT_COMMAND_H

#ifndef ES_HTTP_MULTIPLEXER_EXTENDED_H
#include <ESHttpMultiplexerExtended.h>
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
  virtual ESB::Error run(HttpMultiplexerExtended &multiplexer) = 0;

  virtual const char *name() = 0;

  ESB_DISABLE_AUTO_COPY(HttpClientCommand);
};

}  // namespace ES

#endif
