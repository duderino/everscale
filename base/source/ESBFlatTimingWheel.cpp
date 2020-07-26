#ifndef ESB_FLAT_TIMING_WHEEL_H
#include <ESBFlatTimingWheel.h>
#endif

#ifndef ESB_TIME_H
#include <ESBTime.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ESB {

FlatTimingWheel::FlatTimingWheel(UInt32 ticks, UInt32 tickMilliSeconds, Allocator &allocator)
    : _tickMilliSeconds(tickMilliSeconds),
      _ticks(ticks),
      _currentTick(0U),
      _allocator(allocator),
      _currentTickTime(Time::Instance().now()) {
  _commands = (EmbeddedList *)_allocator.allocate(_ticks * sizeof(EmbeddedList));
  if (!_commands) {
    return;
  }

  for (UInt32 i = 0; i < _ticks; ++i) {
    new (&_commands[i]) EmbeddedList();
  }
}

FlatTimingWheel::~FlatTimingWheel() {
  clear();

  if (_commands) {
    for (UInt32 i = 0; i < _ticks; ++i) {
      _commands[i].~EmbeddedList();
    }
    _allocator.deallocate(_commands);
  }
}

void FlatTimingWheel::clear() {
  for (UInt32 i = 0; i < _ticks; ++i) {
    while (true) {
      EmbeddedListElement *element = _commands[i].removeFirst();
      if (!element) {
        break;
      }

      if (element->cleanupHandler()) {
        element->cleanupHandler()->destroy(element);
      }
    }
  }
}

Error FlatTimingWheel::insert(DelayedCommand *command, UInt32 delayMilliSeconds, bool maskErrors) {
  UInt32 elapsedTicks = ticks(Time::Instance().now() - _currentTickTime);
  UInt32 delayTicks = delayMilliSeconds / _tickMilliSeconds;

  if (elapsedTicks + delayTicks >= _currentTick + _ticks) {
    if (!maskErrors) {
      ESB_LOG_WARNING("%u ticks since last exec + %u requested delay ticks exceeds %u max timing wheel ticks",
                      elapsedTicks, delayTicks, _ticks);
      return ESB_INVALID_ARGUMENT;
    }
    command->setTick(_currentTick + _ticks - 1);
  } else {
    command->setTick(_currentTick + elapsedTicks + delayTicks);
  }

  _commands[idx(command->tick())].addLast(command);
  return ESB_SUCCESS;
}

Error FlatTimingWheel::remove(DelayedCommand *command) {
  _commands[idx(command->tick())].remove(command);
  return 0;
}

Error FlatTimingWheel::run(SharedInt *isRunning) {
  UInt32 elapsedTicks = ticks(Time::Instance().now() - _currentTickTime);

  for (UInt32 maxTick = _currentTick + _ticks - 1; _currentTick <= maxTick; _currentTick++) {
    if (0 == elapsedTicks--) {
      break;
    }

    while (true) {
      Command *command = (Command *)_commands[idx(_currentTick)].removeFirst();
      if (!command) {
        break;
      }
      if (command->run(isRunning)) {
        assert(command->cleanupHandler());
        command->cleanupHandler()->destroy(command);
      }
    }
  }

  _currentTickTime = Time::Instance().now();

  return ESB_SUCCESS;
}

}  // namespace ESB
