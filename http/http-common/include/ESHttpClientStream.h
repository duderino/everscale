#ifndef ES_HTTP_CLIENT_STREAM_H
#define ES_HTTP_CLIENT_STREAM_H

#ifndef ES_HTTP_STREAM_H
#include <ESHttpStream.h>
#endif

namespace ES {

class HttpClientStream : public HttpStream {
 public:
  /**
   * Constructor
   */
  HttpClientStream();

  /**
   * Destructor
   */
  virtual ~HttpClientStream();

 private:
  // Disabled
  HttpClientStream(const HttpClientStream &);
  void operator=(const HttpClientStream &);
};

}  // namespace ES

#endif
