#ifndef ESB_PROCESS_LIMITS_H
#include <ESBProcessLimits.h>
#endif

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

namespace ESB {

static UInt32 GetSoftLimit(int resource) {
#if defined HAVE_STRUCT_RLIMIT && defined HAVE_GETRLIMIT
  struct rlimit rLimit;

  if (0 != getrlimit(resource, &rLimit)) {
    return ESB_UINT32_MAX;
  }

  if (0 >= rLimit.rlim_cur) {
    return ESB_UINT32_MAX;
  }

  if (ESB_UINT32_MAX < rLimit.rlim_cur) {
    return ESB_UINT32_MAX;
  }

  return RLIM_INFINITY == rLimit.rlim_cur ? ESB_UINT32_MAX : rLimit.rlim_cur;
#else
#error "getrlimit() or equivalent is required"
#endif
}

static UInt32 GetHardLimit(int resource) {
#if defined HAVE_STRUCT_RLIMIT && defined HAVE_GETRLIMIT
  struct rlimit rLimit;

  if (0 != getrlimit(resource, &rLimit)) {
    return ESB_UINT32_MAX;
  }

  if (0 >= rLimit.rlim_max) {
    return ESB_UINT32_MAX;
  }

  if (ESB_UINT32_MAX < rLimit.rlim_max) {
    return ESB_UINT32_MAX;
  }

  return RLIM_INFINITY == rLimit.rlim_max ? ESB_UINT32_MAX : rLimit.rlim_max;
#else
#error "getrlimit() or equivalent is required"
#endif
}

static Error SetSoftLimit(int resource, UInt32 limit) {
#if defined HAVE_STRUCT_RLIMIT && defined HAVE_SETRLIMIT && defined HAVE_ERRNO
  struct rlimit rLimit;

  rLimit.rlim_cur = limit;
  rLimit.rlim_max = GetHardLimit(resource);

  if (0 != setrlimit(resource, &rLimit)) {
    return ConvertError(errno);
  }

  return ESB_SUCCESS;
#else
#error "setrlimit() and errno or equivalent is required"
#endif
}

ProcessLimits::ProcessLimits() {}
ProcessLimits::~ProcessLimits() {}

UInt32 ProcessLimits::GetSocketSoftMax() {
  return GetSoftLimit(RLIMIT_NOFILE);
}

UInt32 ProcessLimits::GetSocketHardMax() {
  return GetHardLimit(RLIMIT_NOFILE);
}

Error ProcessLimits::SetSocketSoftMax(UInt32 limit) {
  return SetSoftLimit(RLIMIT_NOFILE, limit);
}

}  // namespace ESB
