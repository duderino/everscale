#ifndef ESB_THREAD_H
#include <ESBThread.h>
#endif

#if !defined HAVE_PTHREAD_YIELD && defined HAVE_SCHED_H && defined HAVE_SCHED_YIELD
#include <sched.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifdef HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

namespace ESB {

void *Thread::ThreadEntry(void *arg) {
  if (!arg) return 0;

  ((Thread *)arg)->run();

  return 0;
}

Thread::Thread() : _isRunning(true) {}

Thread::~Thread() {}

Error Thread::start() {
#ifdef HAVE_PTHREAD_CREATE
  return ConvertError(pthread_create(&_threadId, 0, &ThreadEntry, (Thread *)this));
#else
#error "pthread_create or equivalent is required"
#endif
}

Error Thread::join() {
#ifdef HAVE_PTHREAD_JOIN
  return ConvertError(pthread_join(_threadId, 0));
#else
#error "pthread_join or equivalent is required"
#endif
}

Thread::ThreadId Thread::threadId() const { return _threadId; }

void Thread::Yield() {
#ifdef HAVE_SCHED_YIELD
  sched_yield();
#elif defined HAVE_PTHREAD_YIELD
  pthread_yield();
#else
#error "sched_yield or equivalent is required"
#endif
}

void Thread::Sleep(const Date &date) {
#if defined HAVE_NANOSLEEP && defined HAVE_TIMESPEC_T
  struct timespec timeToSleep;

  timeToSleep.tv_sec = date.seconds();
  timeToSleep.tv_nsec = date.microSeconds() * 1000L;

  nanosleep(&timeToSleep, 0);
#else
#error "nanosleep or equivalent is required"
#endif
}

void Thread::Sleep(long msec) {
#if defined HAVE_NANOSLEEP && defined HAVE_TIMESPEC_T
  struct timespec timeToSleep;

  timeToSleep.tv_sec = msec / 1000;
  timeToSleep.tv_nsec = (msec % 1000) * 1000L * 1000L;

  nanosleep(&timeToSleep, 0);
#else
#error "nanosleep or equivalent is required"
#endif
}

Thread::ThreadId Thread::CurrentThreadId() {
#if defined HAVE_SYSCALL && defined HAVE_GETTID
  return syscall(SYS_gettid);
#elif defined HAVE_PTHREAD_SELF
  return pthread_self();
#else
#error "pthread_self or equivalent is required"
#endif
}

}  // namespace ESB
