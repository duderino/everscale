#ifndef ES_HTTP_STREAM_H
#define ES_HTTP_STREAM_H

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ES_HTTP_REQUEST_H
#include <ESHttpRequest.h>
#endif

#ifndef ES_HTTP_RESPONSE_H
#include <ESHttpResponse.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ES {

class HttpStream {
 public:
  /**
   * Constructor
   */
  HttpStream();

  /**
   * Destructor
   */
  virtual ~HttpStream();

  /**
   * Determine whether the stream uses a secure transport.
   *
   * @return true if the stream uses a secure transport
   */
  virtual bool secure() const = 0;

  /**
   * Abort a stream in any state.
   *
   * @param updateMultiplexer Immediately update registration in the multiplexer
   * @return ESB_SUCCESS if successful, another error code otherwise.   */
  virtual ESB::Error abort(bool updateMultiplexer = true) = 0;

  /**
   * Stop receiving more data on this stream, ultimately applying backpressure
   * to the peer.
   *
   * @param updateMultiplexer Immediately update registration in the multiplexer
   * @return ESB_SUCCESS if successful, another error code otherwise.   */
  virtual ESB::Error pauseRecv(bool updateMultiplexer = true) = 0;

  /**
   * Resume receiving data on this stream.
   *
   * @param updateMultiplexer Immediately update registration in the multiplexer
   * @return ESB_SUCCESS if successful, another error code otherwise.
   * */
  virtual ESB::Error resumeRecv(bool updateMultiplexer = true) = 0;

  /**
   * Stop sending more data on this stream.
   *
   * @param updateMultiplexer Immediately update registration in the multiplexer
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual ESB::Error pauseSend(bool updateMultiplexer = true) = 0;

  /**
   * Resume receiving data on this stream.
   *
   * @param updateMultiplexer Immediately update registration in the multiplexer
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual ESB::Error resumeSend(bool updateMultiplexer = true) = 0;

  /**
   * Use the stream's memory allocator which is suitable for small allocations.
   *
   * @return The stream's memory allocator
   */
  virtual ESB::Allocator &allocator() = 0;

  /**
   * Get the stream's request headers.  This may or may not be populated
   * depending on the stream's state.
   *
   * @return The stream's request object
   */
  virtual const HttpRequest &request() const = 0;

  /**
   * Get the stream's request headers.  This may or may not be populated
   * depending on the stream's state.
   *
   * @return The stream's request object
   */
  virtual HttpRequest &request() = 0;

  /**
   * Get the stream's response headers.  This may or may not be populated
   * depending on the stream's state.
   *
   * @return The stream's response object
   */
  virtual const HttpResponse &response() const = 0;

  /**
   * Get the stream's response headers.  This may or may not be populated
   * depending on the stream's state.
   *
   * @return The stream's response object
   */
  virtual HttpResponse &response() = 0;

  /**
   * Associate an application-specific context with the stream
   *
   * @param context The application-specific context to associate with the
   * stream.
   */
  virtual void setContext(void *context) = 0;

  /**
   * Get the stream's application-specific context.
   *
   * @return Any application-specific context associated with the stream, or
   * NULL if no context has been associated.
   */
  virtual void *context() = 0;

  /**
   * Get the stream's application-specific context.
   *
   * @return Any application-specific context associated with the stream, or
   * NULL if no context has been associated.
   */
  virtual const void *context() const = 0;

  /**
   * Get the address of the stream's peer.
   *
   * @return The stream's peer address
   */
  virtual const ESB::SocketAddress &peerAddress() const = 0;

  /**
   * Get a string that describes the stream for use in log messages.
   *
   * @return A string that describes the stream for use in log messages.
   */
  virtual const char *logAddress() const = 0;

  ESB_DISABLE_AUTO_COPY(HttpStream);
};

}  // namespace ES

#endif
