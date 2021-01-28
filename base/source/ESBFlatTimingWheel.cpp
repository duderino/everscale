#ifndef ESB_FLAT_TIMING_WHEEL_H
#include <ESBFlatTimingWheel.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_TIME_H
#include <ESBTime.h>
#endif

namespace ESB {

FlatTimingWheel::FlatTimingWheel(UInt32 ticks, UInt32 tickMilliSeconds, Allocator &allocator)
    : _start(Time::Instance().now()),
      _tickMilliSeconds(tickMilliSeconds),
      _maxTicks(ticks),
      _currentTick(0U),
      _allocator(allocator) {
  _timers = (EmbeddedList *)_allocator.allocate(_maxTicks * sizeof(EmbeddedList));
  if (!_timers) {
    return;
  }

  for (UInt32 i = 0; i < _maxTicks; ++i) {
    new (&_timers[i]) EmbeddedList();
  }
}

FlatTimingWheel::~FlatTimingWheel() {
  clear();

  if (_timers) {
    for (UInt32 i = 0; i < _maxTicks; ++i) {
      _timers[i].~EmbeddedList();
    }
    _allocator.deallocate(_timers);
  }
}

void FlatTimingWheel::clear() {
  for (UInt32 i = 0; i < _maxTicks; ++i) {
    while (true) {
      EmbeddedListElement *element = _timers[i].removeFirst();
      if (!element) {
        break;
      }

      if (element->cleanupHandler()) {
        element->cleanupHandler()->destroy(element);
      }
    }
  }
}

Error FlatTimingWheel::insert(Timer *timer, UInt32 delayMilliSeconds) {
  if (!timer) {
    return ESB_NULL_POINTER;
  }

  UInt32 nowTick = ticks(Time::Instance().now() - _start);
  UInt32 delayTicks = delayMilliSeconds / _tickMilliSeconds;

  if (nowTick + delayTicks <= _currentTick) {
    return ESB_UNDERFLOW;
  }

  if (nowTick + delayTicks >= _currentTick + _maxTicks) {
    return ESB_OVERFLOW;
  }

  timer->setTick(nowTick + delayTicks);
  _timers[idx(timer->tick())].addLast(timer);
  return ESB_SUCCESS;
}

Error FlatTimingWheel::remove(Timer *timer) {
  if (!timer) {
    return ESB_NULL_POINTER;
  }
  _timers[idx(timer->tick())].remove(timer);
  return ESB_SUCCESS;
}

Timer *FlatTimingWheel::nextExpired() {
  UInt32 nowTick = ticks(Time::Instance().now() - _start);
  if (nowTick == _currentTick) {
    return NULL;
  }

  for (UInt32 tick = _currentTick; tick <= nowTick; ++tick) {
    UInt32 tickIndex = idx(tick);
    Timer *timer = (Timer *)_timers[tickIndex].removeFirst();
    if (timer) {
      if (_timers[tickIndex].isEmpty()) {
        _currentTick = tick;
      }
      return timer;
    }
  }

  _currentTick = nowTick;
  return NULL;
}

}  // namespace ESB
