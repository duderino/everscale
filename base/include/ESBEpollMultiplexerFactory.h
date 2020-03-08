#ifndef ESB_EPOLL_MULTIPLEXER_FACTORY_H
#define ESB_EPOLL_MULTIPLEXER_FACTORY_H

#ifndef ESB_SOCKET_MULTIPLEXER_FACTORY_H
#include <ESBSocketMultiplexerFactory.h>
#endif

#ifndef ESB_EPOLL_MULTIPLEXER_H
#include <ESBEpollMultiplexer.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ESB {

/** A factory that creates epoll socket multiplexers
 *
 * @ingroup network
 */
class EpollMultiplexerFactory : public SocketMultiplexerFactory {
 public:
  /** Constructor.
   *
   * @param name The name of the created multiplexer to be used in log messages.
   * Caller is responsible for the strings memory - use a string literal if
   * possible.
   * @param allocator epoll socket multiplexers will be allocated using this
   * allocator.
   */
  EpollMultiplexerFactory(const char *name, Allocator *allocator);

  /** Destructor.
   */
  virtual ~EpollMultiplexerFactory();

  /** Create a new socket multiplexer
   *
   * @param maxSockets The maxium number of sockets the multiplexer should
   * handle.
   * @return a new socket multiplexer or NULL if it could not be created
   */
  virtual SocketMultiplexer *create(int maxSockets);

  /** Destroy a socket multiplexer
   *
   * @param multiplexer The socket multiplexer to destroy.
   */
  virtual void destroy(SocketMultiplexer *multiplexer);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  //  Disabled
  EpollMultiplexerFactory(const EpollMultiplexerFactory &);
  EpollMultiplexerFactory &operator=(const EpollMultiplexerFactory &);

  const char *_name;
  Allocator *_allocator;
};

}  // namespace ESB

#endif
