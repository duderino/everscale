#ifndef ESB_EVENT_SOCKET_H
#include <ESBEventSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifdef HAVE_SYS_EVENTFD_H
#include <sys/eventfd.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

namespace ESB {

EventSocket::EventSocket() {
#ifdef HAVE_EVENTFD
  _eventFd = eventfd(0, EFD_NONBLOCK);
#else
  // to implement on plaforms without event fd, use a socket or pipe
#error "eventfd() or equivalent is required"
#endif

  if (0 > _eventFd) {
    ESB_LOG_ERROR_ERRNO(ConvertError(_eventFd), "Cannot create event fd");
  }
}

EventSocket::~EventSocket() {
#ifdef HAVE_CLOSE
  close(_eventFd);
  _eventFd = INVALID_SOCKET;
#else
#error "close() or equivalent is required"
#endif
}

Error EventSocket::write(ESB::UInt64 value) {
  if (0 >= value) {
    return ESB_INVALID_ARGUMENT;
  }

#ifdef HAVE_WRITE
  ESB::SSize result = ::write(_eventFd, &value, sizeof(value));
  ESB::Error error = 0 < result ? ESB_SUCCESS : LastError();
#else
#error "write() or equivalent is required"
#endif

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot write to event fd");
  }

  return error;
}

Error EventSocket::read(ESB::UInt64 *value) {
  if (!value) {
    return ESB_NULL_POINTER;
  }

#ifdef HAVE_READ
  ESB::SSize result = ::read(_eventFd, value, sizeof(*value));
  ESB::Error error = 0 < result ? ESB_SUCCESS : LastError();
#else
#error "read() or equivalent is required"
#endif

  if (ESB_AGAIN == error) {
    *value = 0;
    return ESB_SUCCESS;
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot read from event fd");
    return error;
  }

  return ESB_SUCCESS;
}

}  // namespace ESB
