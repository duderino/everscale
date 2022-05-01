#ifndef ES_HTTP_FILTER_H
#define ES_HTTP_FILTER_H

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

namespace ES {

class HttpFilter : public ESB::EmbeddedListElement {
 public:
  HttpFilter();

  virtual ~HttpFilter();

  virtual ESB::CleanupHandler *cleanupHandler();

 private:
  ESB_DEFAULT_FUNCS(HttpFilter);
};

}  // namespace ES

#endif
