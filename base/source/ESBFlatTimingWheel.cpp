#ifndef ESB_FLAT_TIMING_WHEEL_H
#include <ESBFlatTimingWheel.h>
#endif

namespace ESB {

FlatTimingWheel::FlatTimingWheel(UInt32 ticks, UInt32 tickMilliSeconds, const Date &now, Allocator &allocator)
    : _start(now), _tickMilliSeconds(tickMilliSeconds), _maxTicks(ticks), _currentTick(0U), _allocator(allocator) {
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
      Timer *timer = (Timer *)_timers[i].removeFirst();
      if (!timer) {
        break;
      }
      timer->remove();
      if (timer->cleanupHandler()) {
        timer->cleanupHandler()->destroy(timer);
      }
    }
  }
}

Error FlatTimingWheel::insert(Timer *timer, UInt32 delayMilliSeconds, const Date &now) {
  if (!timer) {
    return ESB_NULL_POINTER;
  }

  assert(!timer->inTimingWheel());
  if (timer->inTimingWheel()) {
    return ESB_INVALID_ARGUMENT;
  }

  UInt32 nowTick = ticks(now - _start);
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

  if (!timer->inTimingWheel()) {
    return ESB_INVALID_ARGUMENT;
  }

  _timers[idx(timer->tick())].remove(timer);
  timer->remove();
  return ESB_SUCCESS;
}

Error FlatTimingWheel::update(Timer *timer, UInt32 delayMilliSeconds, const Date &now) {
  if (!timer) {
    return ESB_NULL_POINTER;
  }

  if (!timer->inTimingWheel()) {
    return insert(timer, delayMilliSeconds, now);
  }

  UInt32 nowTick = ticks(now - _start);
  UInt32 delayTicks = delayMilliSeconds / _tickMilliSeconds;

  if (nowTick + delayTicks <= _currentTick) {
    _timers[idx(timer->tick())].remove(timer);
    timer->remove();
    return ESB_UNDERFLOW;
  }

  if (nowTick + delayTicks >= _currentTick + _maxTicks) {
    return ESB_OVERFLOW;
  }

  if (nowTick + delayTicks == timer->tick()) {
    return ESB_SUCCESS;
  }

  _timers[idx(timer->tick())].remove(timer);
  timer->setTick(nowTick + delayTicks);
  _timers[idx(timer->tick())].addLast(timer);
  return ESB_SUCCESS;
}

Timer *FlatTimingWheel::nextExpired(const Date &now) {
  UInt32 nowTick = ticks(now - _start);
  if (nowTick == _currentTick) {
    return NULL;
  }

  for (UInt32 tick = _currentTick; tick <= nowTick; ++tick) {
    UInt32 tickIndex = idx(tick);
    Timer *timer = (Timer *)_timers[tickIndex].removeFirst();
    if (timer) {
      timer->remove();
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
