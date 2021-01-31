#ifndef ESB_TIMER_H
#include <ESBTimer.h>
#endif

namespace ESB {

Timer::Timer() : _tick(-1), _context(NULL) {}

Timer::Timer(void *context) : _tick(-1), _context(context) {}

Timer::~Timer() {}

}  // namespace ESB
