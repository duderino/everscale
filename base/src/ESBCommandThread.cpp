#ifndef ESB_COMMAND_THREAD_H
#include <ESBCommandThread.h>
#endif

namespace ESB {

CommandThread::CommandThread(Command *command) : _command(command) {}

CommandThread::~CommandThread() {}

void CommandThread::run() {
  if (!_command) {
    return;
  }

  if (_command->run(&_isRunning)) {
    CleanupHandler *cleanupHandler = _command->getCleanupHandler();

    if (cleanupHandler) {
      cleanupHandler->destroy(_command);
    }
  }
}

}  // namespace ESB
