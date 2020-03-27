#ifndef ESB_SOCKET_MULTIPLEXER_FACTORY_H
#define ESB_SOCKET_MULTIPLEXER_FACTORY_H

#ifndef ESB_SOCKET_MULTIPLEXER_H
#include <ESBSocketMultiplexer.h>
#endif

namespace ESB {

/** An abstract factory that creates socket multiplexers
 *
 * @ingroup network
 */
class SocketMultiplexerFactory {
 public:
  /** Constructor.
   */
  SocketMultiplexerFactory();

  /** Destructor.
   */
  virtual ~SocketMultiplexerFactory();

  /** Create a new socket multiplexer
   *
   * @param maxSockets The maximum number of sockets the multiplexer should
   * handle.
   * @return a new socket multiplexer or NULL if it could not be created
   */
  virtual SocketMultiplexer *create(int maxSockets) = 0;

  /** Destroy a socket multiplexer
   *
   * @param multiplexer The socket multiplexer to destroy.
   */
  virtual void destroy(SocketMultiplexer *multiplexer) = 0;

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 private:
  //  Disabled
  SocketMultiplexerFactory(const SocketMultiplexerFactory &);
  SocketMultiplexerFactory &operator=(const SocketMultiplexerFactory &);
};

}  // namespace ESB

#endif
