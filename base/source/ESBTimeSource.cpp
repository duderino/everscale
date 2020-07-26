#ifndef ESB_TIME_SOURCE_H
#include <ESBTimeSource.h>
#endif

namespace ESB {

TimeSource::TimeSource() {}

TimeSource::~TimeSource() {}

FakeTimeSource::FakeTimeSource(const Date &date) : _now(date) {}

FakeTimeSource::~FakeTimeSource() {}

Date FakeTimeSource::now() { return _now; }

}  // namespace ESB
