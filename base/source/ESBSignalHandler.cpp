#ifndef ESB_SIGNAL_HANDLER_H
#include <ESBSignalHandler.h>
#endif

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_CXXABI_H
#include <cxxabi.h>
#endif

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_UCONTEXT_H
#include <ucontext.h>
#endif

#ifndef ESB_MAX_BACKTRACE_FRAMES
#define ESB_MAX_BACKTRACE_FRAMES 42
#endif

namespace ESB {

static volatile Word Running = 1;
static const UInt32 BacktraceSignals[] = {SIGILL, SIGFPE, SIGABRT, SIGSEGV, SIGBUS, SIGSYS};
static const UInt32 StopSignals[] = {SIGINT, SIGTERM, SIGQUIT};
static const UInt32 IgnoreSignals[] = {SIGHUP,  SIGPIPE, SIGURG,    SIGTTIN, SIGTTOU, SIGPOLL,
                                       SIGXCPU, SIGXFSZ, SIGVTALRM, SIGUSR1, SIGUSR2, SIGWINCH};

static const char *DescribeSignal(int signo) {
  if (SIGILL > signo || _NSIG <= signo) {
    return "Unknown Signal";
  }

#if defined HAVE_SIGDESCR_NP
  return sigdescr_np(signo);
#elif defined HAVE_SYS_SIGLIST
  return sys_siglist[signo];
#else
#error "sigdescr_np or equivalent is required for the signal handler"
#endif

}

void BacktraceHandler(int signo, siginfo_t *siginfo, void *context) {
  //
  // TSAN is disabled because this function makes hearty use of code that is not async-signal-safe.  While normally
  // that'd be a very bad thing, this handler is only called when the process is about to exit.  Normally it will log
  // extra info that makes it much easier to debug a crash.  Occasionally it may crash a progress that was just about to
  // exit/crash anyways.
  //

  if (!ESB_CRITICAL_LOGGABLE) {
    signal(signo, SIG_DFL);
    raise(signo);
  }

#if defined(__has_feature) && !__has_feature(thread_sanitizer)

  const char *description = DescribeSignal(signo);

#if defined HAVE_BACKTRACE && defined HAVE_BACKTRACE_SYMBOLS && defined HAVE_ABI_CXA_DEMANGLE && defined HAVE_DLADDR
  //
  // From "Stack Backtracing Inside Your Program" by Gianluca Insolvibile on August 11, 2003, Linux Journal.  Retrieved
  // on Jan 18th 2021 from https://www.linuxjournal.com/article/6391.
  //
  void *frames[ESB_MAX_BACKTRACE_FRAMES];
  int numFrames = backtrace(frames, ESB_MAX_BACKTRACE_FRAMES);

#ifdef ESB_RESTORE_FIRST_BACKTRACE_FRAME
  const int startFrame = 1;
#ifdef HAVE_UCONTEXT_T
  {
    ucontext_t *uc = (ucontext_t *)_context;
#ifdef ESB_64BIT
    frames[1] = (void *)uc->uc_mcontext.gregs[REG_RIP];
#else
    frames[1] = (void *)uc->uc_mcontext.gregs[REG_EIP];
#endif
  }
#else
#error "ucontext_t or equivalent is required"
#endif
#else
  const int startFrame = 2;
#endif

  char **symbols = backtrace_symbols(frames, numFrames);

  for (int i = startFrame; i < numFrames; ++i) {
    const char *mangledFrame = symbols[i] ? symbols[i] : "(null)";
    Dl_info info;
    memset(&info, 0, sizeof(info));
    if (dladdr(frames[i], &info)) {
      int status = 0;
      mangledFrame = info.dli_sname ? info.dli_sname : mangledFrame;
      char *demangledFrame = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
      ESB_LOG_BACKTRACE("[%s:%2d/%d]: %s", description, i - startFrame + 1, numFrames - startFrame,
                        0 == status ? (demangledFrame ? demangledFrame : mangledFrame) : mangledFrame);
      if (demangledFrame) {
        free(demangledFrame);
      }
    } else {
      ESB_LOG_BACKTRACE("[%s:%2d/%d]: %s", description, i - startFrame + 1, numFrames - startFrame, mangledFrame);
    }
  }

  if (symbols) {
    free(symbols);
  }
#else
#error "backtrace, backtrace_symbols, abi::__cxa_demangle, and dladdr or equivalent is required"
#endif

  ESB::Logger::Instance().flush();

#endif  // !__has_feature(thread_sanitizer)

  signal(signo, SIG_DFL);
  raise(signo);
}

static void StopHandler(int signo, siginfo_t *siginfo, void *context) { Running = 0; }

Error SignalHandler::initialize() {
  for (UInt32 i = 0; i < sizeof(BacktraceSignals) / sizeof(UInt32); ++i) {
#if defined HAVE_SIGACTION && defined HAVE_STRUCT_SIGACTION
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = BacktraceHandler;
    sa.sa_flags = SA_RESTART | SA_SIGINFO;

    if (0 != sigaction(BacktraceSignals[i], &sa, NULL)) {
      Error error = LastError();
      ESB_LOG_ERROR_ERRNO(error, "Cannot install sighandler for signo %d (%s)", BacktraceSignals[i],
                          DescribeSignal(BacktraceSignals[i]));
      return error;
    }
#else
#error "struct sigaction and sigaction or equivalent are required"
#endif
  }

  for (UInt32 i = 0; i < sizeof(StopSignals) / sizeof(UInt32); ++i) {
#if defined HAVE_SIGACTION && defined HAVE_STRUCT_SIGACTION
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = StopHandler;
    sa.sa_flags = SA_RESTART | SA_SIGINFO;

    if (0 != sigaction(StopSignals[i], &sa, NULL)) {
      Error error = LastError();
      ESB_LOG_ERROR_ERRNO(error, "Cannot install sighandler for signo %d (%s)", StopSignals[i],
                          DescribeSignal(StopSignals[i]));
      return error;
    }
#else
#error "struct sigaction and sigaction or equivalent are required"
#endif
  }

  for (ESB::UInt32 i = 0; i < sizeof(IgnoreSignals) / sizeof(ESB::UInt32); ++i) {
#ifdef HAVE_SIGIGNORE
    if (0 != sigignore(IgnoreSignals[i])) {
      Error error = LastError();
      ESB_LOG_ERROR_ERRNO(error, "Cannot ignore signo %d (%s)", IgnoreSignals[i], DescribeSignal(IgnoreSignals[i]));
      return error;
    }
#else
#error "sigignore or equivalent is required"
#endif
  }

  return ESB_SUCCESS;
}

SignalHandler SignalHandler::_Instance;

SignalHandler::SignalHandler() {}

SignalHandler::~SignalHandler() {}

bool SignalHandler::running() { return Running; }

void SignalHandler::stop() { Running = 0; }

}  // namespace ESB
