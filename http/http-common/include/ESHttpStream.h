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
   * Is the stream paused?  Paused streams cannot receive more data until they
   * are unpaused.
   *
   * @return true if the stream is paused, false otherwise.
   */
  virtual bool isPaused() = 0;

  /**
   * Resumed a paused stream.  Note: your handler implementation must be
   * reentrant if you call this function because calling this function may in
   * turn call other handle* functions including handleRemove.
   *
   * @return ESB_SUCCESS if successfully resumed, another error code otherwise.
   */
  virtual ESB::Error resume() = 0;

  /**
   * Cancel a paused stream.  Note: your handler implementation must be
   * reentrant if you call this function because calling this function may in
   * turn call other handle* functions including handleRemove.
   *
   * @return ESB_SUCCESS if successfully canceled, another error code otherwise.
   */
  virtual ESB::Error cancel() = 0;

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

 private:
  // Disabled
  HttpStream(const HttpStream &);
  void operator=(const HttpStream &);
};

}  // namespace ES

#endif
