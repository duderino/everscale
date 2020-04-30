#ifndef ES_HTTP_SERVER_STREAM_H
#define ES_HTTP_SERVER_STREAM_H

#ifndef ES_HTTP_STREAM_H
#include <ESHttpStream.h>
#endif

namespace ES {

class HttpServerStream : public HttpStream {
 public:
  /**
   * Constructor
   */
  HttpServerStream();

  /**
   * Destructor
   */
  virtual ~HttpServerStream();

 private:
  // Disabled
  HttpServerStream(const HttpServerStream &);
  void operator=(const HttpServerStream &);
};

}  // namespace ES

#endif
