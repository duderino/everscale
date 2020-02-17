#ifndef ESB_SOCKET_H
#define ESB_SOCKET_H

/** @defgroup network Network */

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#ifndef HAVE_SOCKET_T
typedef int SOCKET;
#endif

#ifndef HAVE_DECL_INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#ifndef HAVE_DECL_SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#endif
