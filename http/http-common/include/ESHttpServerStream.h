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

  /**
   * Send a response with an empty body
   *
   * @param statusCode the HTTP status code
   * @param reasonPhrase the HTTP reason phrase
   * @return ESB_SUCCESS if successful, ESB_AGAIN if send buffer full and
   * underlying socket send buffer is full, another error code otherwise.
   */
  virtual ESB::Error sendEmptyResponse(int statusCode, const char *reasonPhrase) = 0;

  /**
   * Send a response
   *
   * @param response The response headers to send (a copy will be made of this)
   * @return ESB_SUCCESS if successful, ESB_AGAIN if send buffer full and
   * underlying socket send buffer is full, another error code otherwise.
   */
  virtual ESB::Error sendResponse(const HttpResponse &response) = 0;

  /**
   * Buffer and occasionally flush a request body chunk to the underlying
   * socket.
   *
   * @param body The chunk to send
   * @param bytesOffered The size of the chunk to send.  If 0, this is the last
   * chunk, so immediately flush all buffered bytes to the underlying socket.
   * @param bytesConsumed The number of bytes consumed
   * @return ESB_SUCCESS if 1+ bytes consumed, ESB_AGAIN if send buffer full and
   * underlying socket send buffer is full, another error code otherwise.
   */
  virtual ESB::Error sendResponseBody(unsigned const char *body, ESB::UInt32 bytesOffered,
                                      ESB::UInt32 *bytesConsumed) = 0;

  /**
   * Determine how many bytes of request body data can be read without blocking.
   * This may trigger a read from the underlying socket as a side effect.
   *
   * @param bytesAvailable The number of bytes that can be consumed
   * @return ESB_SUCCESS + 0 means the last chunk is ready to be read,
   * ESB_SUCCESS + 1+ means body data can be read, ESB_AGAIN means buffers are
   * empty and there is no data to be read on the underlying socket, or another
   * error code otherwise.
   */
  virtual ESB::Error requestBodyAvailable(ESB::UInt32 *bytesAvailable) = 0;

  /**
   * Read up to bytesRequested of request body data.
   *
   * @param body Data should be written here
   * @param bytesRequested The amount of data to write.  This must be <= the
   * bytesAvailable result returned by requestBodyAvailable().
   * @paran bytesRead The bytes actually written to the body buffer.
   * @return ESB_SUCCESS and byteRead == bytesRequested if successful, ESB_AGAIN and a bytesRead >= 0 may also be
   * returned, ESB_INVALID_ARGUMENT if bytesRequested exceeds bytesAvailable (use requestBodyAvailable() first), another
   * error code otherwise.
   */
  virtual ESB::Error readRequestBody(unsigned char *body, ESB::UInt32 bytesRequested, ESB::UInt32 *bytesRead) = 0;

 private:
  // Disabled
  HttpServerStream(const HttpServerStream &);
  void operator=(const HttpServerStream &);
};

}  // namespace ES

#endif
