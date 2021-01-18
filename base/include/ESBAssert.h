#ifndef ESB_ASSERT_H
#define ESB_ASSERT_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#ifndef HAVE_BACKTRACE
#error "backtrace or equivalent is required"
#endif

#ifndef HAVE_BACKTRACE_SYMBOLS
#error "backtrace_symbols or equivalent is required"
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_CXXABI_H
#include <cxxabi.h>
#endif

#ifndef HAVE_ABI_CXA_DEMANGLE
#error "abi::__cxa_demangle or equivalent is required"
#endif

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#ifndef HAVE_DLADDR
#error "dladdr or equivalent is required"
#endif

#ifndef ESB_MAX_BACKTRACE_FRAMES
#define ESB_MAX_BACKTRACE_FRAMES 42
#endif

#ifdef NDEBUG
#define ESB_ASSERT(expr) assert(expr)
#else
#define ESB_ASSERT(expr)                                                                                       \
  if (!(expr)) {                                                                                               \
    if (ESB_ERROR_LOGGABLE) {                                                                                  \
      void *esb_bt_buffer[ESB_MAX_BACKTRACE_FRAMES];                                                           \
      int esb_bt_frames = backtrace(esb_bt_buffer, ESB_MAX_BACKTRACE_FRAMES);                                  \
      for (int i = 0; i < esb_bt_frames; ++i) {                                                                \
        Dl_info esb_bt_info;                                                                                   \
        if (dladdr(esb_bt_buffer[i], &esb_bt_info)) {                                                          \
          int esb_bt_status = 0;                                                                               \
          char *esb_bt_demangled = abi::__cxa_demangle(esb_bt_info.dli_sname, NULL, 0, &esb_bt_status);        \
          ESB_LOG_ERROR("%s: %d %s", #expr, i, 0 == esb_bt_status ? esb_bt_demangled : esb_bt_info.dli_sname); \
          if (esb_bt_demangled) free(esb_bt_demangled);                                                        \
        } else {                                                                                               \
          char **esb_bt_symbols = backtrace_symbols(&esb_bt_buffer[i], 1);                                     \
          ESB_LOG_ERROR("%s: %d %s", #expr, i, esb_bt_symbols ? esb_bt_symbols[0] : "(null)");                 \
          if (esb_bt_symbols) free(esb_bt_symbols);                                                            \
        }                                                                                                      \
      }                                                                                                        \
      ESB::Logger::Instance().flush();                                                                         \
    }                                                                                                          \
    assert((expr));                                                                                            \
  }
#endif
#endif