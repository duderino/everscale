/** @file ESFSocket.h
 *  @brief Definitions of socket descriptors, invalid sockets, and socket
 *      errors
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_SOCKET_H
#define ESF_SOCKET_H

/** @defgroup network Network */

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
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

#endif /* ! ESF_SOCKET_H */
