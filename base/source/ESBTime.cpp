#ifndef ESB_TIME_H
#include <ESBTime.h>
#endif

#ifndef ESB_SYSTEM_TIME_SOURCE_H
#include <ESBSystemTimeSource.h>
#endif

namespace ESB {

Time Time::_Instance;

Time::Time() : _source(&SystemTimeSource::Instance()) {}
Time::~Time() {}

}  // namespace ESB
