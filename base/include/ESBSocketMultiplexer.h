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

namespace ESB {

class MultiplexedSocket;

/** A command that delegates i/o readiness events to multiple
 * MultiplexedSockets.  The command can be run in the current thread of
 * control or run by an CommandThread or even in an ThreadPool.
 *
 * @ingroup network
 */
class SocketMultiplexer : public Command {
 public:
  /** Constructor.
   */
  SocketMultiplexer();

  /** Destructor.
   */
  virtual ~SocketMultiplexer();

  /** Add a new multiplexed socket to the socket multiplexer
   *
   * @param multiplexedSocket The multiplexed socket to add
   * @return ESB_SUCCESS if successful, ESB_OVERFLOW if the maximum sockets this
   *  multiplexer can handle has been reached, another error code otherwise.
   */
  virtual Error addMultiplexedSocket(MultiplexedSocket *multiplexedSocket) = 0;

  /** Keep socket in epoll and socket list, but possibly change the readiness
   *  events of interest.  This does not modify the _activeSocketCount.
   *
   * @param socket The multiplexedSocket
   */
  virtual Error updateMultiplexedSocket(MultiplexedSocket *socket) = 0;

  /** Remove a multiplexed socket from the socket multiplexer
   *
   * @param socket The multiplexed socket to remove
   */
  virtual Error removeMultiplexedSocket(MultiplexedSocket *socket) = 0;

  /** Get the number of sockets this multiplexer is currently handling.
   *
   * @return the number of sockets this multiplexer is currently handling.
   */
  virtual int currentSockets() const = 0;

  /** Get the maximum number of sockets this multiplexer can handle.
   *
   * @return the maximum number of sockets this multiplexer can handle.
   */
  virtual int maximumSockets() const = 0;

  /** Determine whether this multiplexer has been shutdown.
   *
   * @return true if the multiplexer should still run, false if it has been
   *   told to shutdown.
   */
  virtual bool isRunning() const = 0;

 private:
  //  Disabled
  SocketMultiplexer(const SocketMultiplexer &);
  SocketMultiplexer &operator=(const SocketMultiplexer &);
};

}  // namespace ESB

#endif
