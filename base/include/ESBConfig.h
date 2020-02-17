#ifndef ESB_CONFIG_H
#define ESB_CONFIG_H

#ifdef ES_LINUX
#include <ESBLinuxConfig.h>
#elif defined ES_SOLARIS
#include <ESBSolarisConfig.h>
#elif defined ES_MACOS
#include <ESBMacOSConfig.h>
#elif defined ES_WIN32
#include <ESBWin32Config.h>
#else
#include <config.h>
#endif

#endif
