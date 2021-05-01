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
  virtual ESB::Error sendRequestBody(unsigned const char *body, ESB::UInt64 bytesOffered,
                                     ESB::UInt64 *bytesConsumed) = 0;

  /**
   * Determine how many bytes of response body data can be read without
   * blocking.  This may trigger a read from the underlying socket as a side
   * effect.
   *
   * @param bytesAvailable The number of bytes that can be consumed
   * @return ESB_SUCCESS + 0 means the last chunk is ready to be read,
   * ESB_SUCCESS + 1+ means body data can be read, ESB_AGAIN means buffers are
   * empty and there is no data to be read on the underlying socket, or another
   * error code otherwise.
   */
  virtual ESB::Error responseBodyAvailable(ESB::UInt64 *bytesAvailable) = 0;

  /**
   * Read up to bytesRequested of response body data.
   *
   * @param body Data should be written here
   * @param bytesRequested The amount of data to write.  This must be <= the bytesAvailable result returned by
   * responseBodyAvailable().
   * @paran bytesRead The bytes actually written to the body buffer.
   * @return ESB_SUCCESS and bytesRead == bytesRequested if successful, ESB_AGAIN and a bytesRead >= 0 may also be
   * returned, ESB_INVALID_ARGUMENT if bytesRequested exceeds bytesAvailable (use requestBodyAvailable() first), another
   * error code otherwise.
   */
  virtual ESB::Error readResponseBody(unsigned char *body, ESB::UInt64 bytesRequested, ESB::UInt64 *bytesRead) = 0;

 private:
  // Disabled
  HttpClientStream(const HttpClientStream &);
  void operator=(const HttpClientStream &);
};

}  // namespace ES

#endif
