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
   * @param chunk The chunk to send
   * @param bytesOffered The size of the chunk to send.  If 0, this is the last
   * chunk, so immediately flush all buffered bytes to the underlying socket.
   * @param bytesConsumed The number of bytes consumed
   * @return ESB_SUCCESS if 1+ bytes consumed, ESB_AGAIN if send buffer full and
   * underlying socket send buffer is full, another error code otherwise.
   */
  virtual ESB::Error sendRequestBody(unsigned const char *chunk, ESB::UInt32 bytesOffered,
                                     ESB::UInt32 *bytesConsumed) = 0;

  /**
   * Determine how many bytes of response body data can be read without
   * blocking.  This may trigger a read from the underlying socket as a side
   * effect.
   *
   * @param bytesAvailable The number of bytes that can be consumed
   * @param bufferOffset A value which should be passed to the
   * readResponseBody() function.
   * @return ESB_SUCCESS + 0 means the last chunk is ready to be read,
   * ESB_SUCCESS + 1+ means body data can be read, ESB_AGAIN means buffers are
   * empty and there is no data to be read on the underlying socket, or another
   * error code otherwise.
   */
  virtual ESB::Error responseBodyAvailable(ESB::UInt32 *bytesAvailable, ESB::UInt32 *bufferOffset) = 0;

  /**
   * Read up to bytesRequested of response body data.
   *
   * @param chunk Data should be written here
   * @param bytesRequested The amount of data to write.  This must be <= the
   * bytesAvailable result returned by responseBodyAvailable().
   * @param bufferOffset A value read from the responseBodyAvailable() function
   * @return ESB_SUCCESS if successful, ESB_INVALID_ARGUMENT if bytesRequested
   * exceeds bytesAvailable, another error code otherwise.
   */
  virtual ESB::Error readResponseBody(unsigned char *chunk, ESB::UInt32 bytesRequested, ESB::UInt32 bufferOffset) = 0;

 private:
  // Disabled
  HttpClientStream(const HttpClientStream &);
  void operator=(const HttpClientStream &);
};

}  // namespace ES

#endif
