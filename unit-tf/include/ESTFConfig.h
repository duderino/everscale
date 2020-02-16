#ifndef ESTF_CONFIG_H
#define ESTF_CONFIG_H

/** @defgroup unit-test Unit Test Framework */

#ifdef ES_LINUX
#include <ESTFLinuxConfig.h>
#elif defined ES_SOLARIS
#include <ESTFSolarisConfig.h>
#elif defined ES_MACOS
#include <ESTFMacOSConfig.h>
#else
#include <config.h>
#endif

#endif
