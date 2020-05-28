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
  virtual ESB::Error sendResponseBody(unsigned const char *chunk,
                                      ESB::UInt32 bytesOffered,
                                      ESB::UInt32 *bytesConsumed) = 0;

  /**
   * Determine how many bytes of request body data can be read without blocking.
   * This may trigger a read from the underlying socket as a side effect.
   *
   * @param bytesAvailable The number of bytes that can be consumed
   * @param bufferOffset A value which should be passed to the readRequestBody()
   * function.
   * @return ESB_SUCCESS + 0 means the last chunk is ready to be read,
   * ESB_SUCCESS + 1+ means body data can be read, ESB_AGAIN means buffers are
   * empty and there is no data to be read on the underlying socket, or another
   * error code otherwise.
   */
  virtual ESB::Error requestBodyAvailable(ESB::UInt32 *bytesAvailable,
                                          ESB::UInt32 *bufferOffset) = 0;

  /**
   * Read up to bytesRequested of request body data.
   *
   * @param chunk Data should be written here
   * @param bytesRequested The amount of data to write.  This must be <= the
   * bytesAvailable result returned by requestBodyAvailable().
   * @param bufferOffset A value read from the requestBodyAvailable() function
   * @return ESB_SUCCESS if successful, ESB_INVALID_ARGUMENT if bytesRequested
   * exceeds bytesAvailable, another error code otherwise.
   */
  virtual ESB::Error readRequestBody(unsigned char *chunk,
                                     ESB::UInt32 bytesRequested,
                                     ESB::UInt32 bufferOffset) = 0;

 private:
  // Disabled
  HttpServerStream(const HttpServerStream &);
  void operator=(const HttpServerStream &);
};

}  // namespace ES

#endif
