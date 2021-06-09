#ifndef ESB_EVENT_SOCKET_H
#define ESB_EVENT_SOCKET_H

#ifndef ESB_SOCKET_TYPE_H
#include <ESBSocketType.h>
#endif

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
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

 private:
  SOCKET _eventFd;

  ESB_DEFAULT_FUNCS(EventSocket);
};

}  // namespace ESB

#endif
