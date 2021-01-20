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
SignalHandler SignalHandler::_Instance;
static void BacktraceHandler(int signo, siginfo_t *siginfo, void *context);
static void StopHandler(int signo, siginfo_t *siginfo, void *context);
static const char *DescribeSignal(int signo);

static const UInt32 BacktraceSignals[] = {SIGILL, SIGFPE, SIGABRT, SIGSEGV, SIGBUS, SIGSYS};
static const UInt32 StopSignals[] = {SIGINT, SIGTERM, SIGQUIT};
static const UInt32 IgnoreSignals[] = {SIGHUP,  SIGPIPE, SIGURG,    SIGTTIN, SIGTTOU, SIGPOLL,
                                       SIGXCPU, SIGXFSZ, SIGVTALRM, SIGUSR1, SIGUSR2, SIGWINCH};

SignalHandler::SignalHandler() {}

SignalHandler::~SignalHandler() {}

bool SignalHandler::running() { return Running; }

void SignalHandler::stop() { Running = 0; }

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

void BacktraceHandler(int signo, siginfo_t *siginfo, void *context) {
  if (!ESB_CRITICAL_LOGGABLE) {
    exit(0 == signo ? 1 : signo);
  }

  const char *description = DescribeSignal(signo);

#if defined HAVE_BACKTRACE && defined HAVE_BACKTRACE_SYMBOLS && defined HAVE_ABI_CXA_DEMANGLE && defined HAVE_DLADDR
  //
  // From "Stack Backtracing Inside Your Program" by Gianluca Insolvibile on August 11, 2003, Linux Journal.  Retrieved
  // on Jan 18th 2021 from https://www.linuxjournal.com/article/6391.
  //
  void *frames[ESB_MAX_BACKTRACE_FRAMES];
  int numFrames = backtrace(frames, ESB_MAX_BACKTRACE_FRAMES);

#ifdef HAVE_UCONTEXT_T
  {
    // TODO not sure if this trick from the linux journal article is still useful.  Perhaps start the log dump from i =
    // 2 (third frame) instead?
    ucontext_t *uc = (ucontext_t *)context;
#ifdef ESB_64BIT
    frames[1] = (void *)uc->uc_mcontext.gregs[REG_RIP];
#else
    frames[1] = (void *)uc->uc_mcontext.gregs[REG_EIP];
#endif
  }
#else
#error "ucontext_t or equivalent is required"
#endif

  char **symbols = backtrace_symbols(frames, numFrames);

  for (int i = 1; i < numFrames; ++i) {
    const char *mangledFrame = symbols[i] ? symbols[i] : "(null)";
    Dl_info info;
    memset(&info, 0, sizeof(info));
    if (dladdr(frames[i], &info)) {
      int status = 0;
      mangledFrame = info.dli_sname ? info.dli_sname : mangledFrame;
      char *demangledFrame = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
      ESB_LOG_CRITICAL("(%d:%s:%d): %s", signo, description, i,
                       0 == status ? (demangledFrame ? demangledFrame : mangledFrame) : mangledFrame);
      if (demangledFrame) {
        free(demangledFrame);
      }
    } else {
      ESB_LOG_CRITICAL("(%d:%s:%d): %s", signo, description, i, mangledFrame);
    }
  }

  if (symbols) {
    free(symbols);
  }
#else
#error "backtrace, backtrace_symbols, abi::__cxa_demangle, and dladdr or equivalent is required"
#endif

  ESB::Logger::Instance().flush();
  exit(0 == signo ? 1 : signo);
}

void StopHandler(int signo, siginfo_t *siginfo, void *context) { Running = 0; }

const char *DescribeSignal(int signo) {
  if (SIGILL > signo || _NSIG <= signo) {
    return "Unknown Signal";
  }

  return sys_siglist[signo];
}

}  // namespace ESB
