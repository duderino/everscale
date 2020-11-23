#ifndef ESB_EVENT_SOCKET_H
#define ESB_EVENT_SOCKET_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_SOCKET_TYPE_H
#include <ESBSocketType.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ESB {

/** A socket that can wake up multiplexers for non-networking events.
 *
 *  @ingroup network
 */
class EventSocket {
 public:
  /** Constructor
   */
  EventSocket();

  /** Destructor.
   */
  virtual ~EventSocket();

  /**
   * Add 1+ to the EventSocket's event count
   *
   * @param value a positive value to add to the event count
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error write(ESB::UInt64 value);

  /**
   * Read and consume the EventSocket's event count.  The EventSocket's
   * event count will (however briefly) be set to 0 after this completes.
   *
   * @param value the value read from the event socket's event count
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error read(ESB::UInt64 *value);

  /** Get the socket's socket descriptor.
   *
   *  @return the socket descriptor
   */
  inline SOCKET socketDescriptor() const { return _eventFd; }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  // Disabled
  EventSocket(const EventSocket &);
  EventSocket &operator=(const EventSocket &);

  SOCKET _eventFd;
};

}  // namespace ESB

#endif
