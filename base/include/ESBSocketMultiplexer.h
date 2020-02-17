#ifndef ESB_SOCKET_MULTIPLEXER_H
#define ESB_SOCKET_MULTIPLEXER_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_COMMAND_H
#include <ESBCommand.h>
#endif

#ifndef ESB_MULTIPLEXED_SOCKET_H
#include <ESBMultiplexedSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ESB {

/** A command that delegates i/o readiness events to multiple
 * MultiplexedSockets.  The command can be run in the current thread of
 * control or run by an CommandThread or even in an ThreadPool.
 *
 * @ingroup network
 */
class SocketMultiplexer : public Command {
 public:
  /** Constructor.
   *
   * @param name The name of the multiplexer to be used in log messages.  Caller
   * is responsible for the strings memory - use a string literal if possible.
   * @param logger An optional logger.  Pass NULL to not log anything.
   */
  SocketMultiplexer(const char *name, Logger *logger);

  /** Destructor.
   */
  virtual ~SocketMultiplexer();

  /** Initialize the multiplexer
   *
   * @return ESB_SUCCESS if successful, another error code otherwise
   */
  virtual Error initialize() = 0;

  /** Destroy the multiplexer
   *
   */
  virtual void destroy() = 0;

  /** Add a new multiplexed socket to the socket multiplexer
   *
   * @param multiplexedSocket The multiplexed socket to add
   * @return ESB_SUCCESS if successful, ESB_OVERFLOW if the maximum sockets this
   *  multiplexer can handle has been reached, another error code otherwise.
   */
  virtual Error addMultiplexedSocket(MultiplexedSocket *multiplexedSocket) = 0;

  /** Get the number of sockets this multiplexer is currently handling.
   *
   * @return the number of sockets this multiplexer is currently handling.
   */
  virtual int getCurrentSockets() = 0;

  /** Get the maximum number of sockets this multiplexer can handle.
   *
   * @return the maximum number of sockets this multiplexer can handle.
   */
  virtual int getMaximumSockets() = 0;

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

 protected:
  const char *_name;
  Logger *_logger;

 private:
  //  Disabled
  SocketMultiplexer(const SocketMultiplexer &);
  SocketMultiplexer &operator=(const SocketMultiplexer &);
};

}  // namespace ESB

#endif
