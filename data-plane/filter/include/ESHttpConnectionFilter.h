#ifndef ES_HTTP_CONNECTION_FILTER_H
#define ES_HTTP_CONNECTION_FILTER_H

#ifndef ES_HTTP_FILTER_H
#include <ESHttpFilter.h>
#endif

namespace ES {

class HttpConnectionFilter : public HttpFilter {
 public:
  HttpConnectionFilter();

  virtual ~HttpConnectionFilter();

 private:
  ESB_DEFAULT_FUNCS(HttpConnectionFilter);
};

}  // namespace ES

#endif
