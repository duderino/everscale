#ifndef ES_HTTP_COMMAND_SOCKET_H
#define ES_HTTP_COMMAND_SOCKET_H

#ifndef ESB_MULTIPLEXED_SOCKET_H
#include <ESBMultiplexedSocket.h>
#endif

#ifndef ESB_EVENT_SOCKET_H
#include <ESBEventSocket.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#define ESB_COMMAND_SUFFIX "-command"
#define ESB_COMMAND_SUFFIX_SIZE 9

namespace ES {

/** A base class for sockets that can wake up multiplexers to run commands.
 */
class HttpCommandSocket : public ESB::MultiplexedSocket {
 public:
  /** Constructor
   */
  HttpCommandSocket(const char *namePrefix);

  /** Destructor.
   */
  virtual ~HttpCommandSocket();

  virtual const void *key() const;

  //
  // ESB::MultiplexedSocket
  //

  virtual bool wantAccept();

  virtual bool wantConnect();

  virtual bool wantRead();

  virtual bool wantWrite();

  virtual bool isIdle();

  virtual ESB::Error handleAccept();

  virtual ESB::Error handleConnect();

  virtual ESB::Error handleReadable();

  virtual ESB::Error handleWritable();

  virtual void handleError(ESB::Error errorCode);

  virtual void handleRemoteClose();

  virtual void handleIdle();

  virtual bool handleRemove();

  virtual SOCKET socketDescriptor() const;

  virtual ESB::CleanupHandler *cleanupHandler();

  virtual const char *name() const;

 protected:
  /**
   * Enqueue a command on the command socket.  When the command socket is
   * in a multiplexer, the multiplexer will wake up, dequeue the command,
   * and execute it on the multiplexer's thread of control.
   *
   * @param command The command to execute
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error pushInternal(ESB::EmbeddedListElement *command);

  // Subclasses must implement this to downcast EmbeddedListElement to either
  // HttpClientCommand or HttpServerCommand
  virtual ESB::Error runCommand(ESB::EmbeddedListElement *command) = 0;

 private:
  // Disabled
  HttpCommandSocket(const HttpCommandSocket &);
  HttpCommandSocket &operator=(const HttpCommandSocket &);

  ESB::EventSocket _eventSocket;
  ESB::Mutex _lock;
  ESB::EmbeddedList _queue;
  bool _removed;
  char _name[ESB_NAME_PREFIX_SIZE + ESB_COMMAND_SUFFIX_SIZE];
};

}  // namespace ES

#endif
