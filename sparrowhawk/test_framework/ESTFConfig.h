/** @file ESTFConfig.h
 *  @brief This file can be included to define the target platform's properties.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 * *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:19 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESTF_CONFIG_H
#define ESTF_CONFIG_H

/** @defgroup test Test Framework */

using namespace std;

#ifdef ES_LINUX
#include <ESTFLinuxConfig.h>
#elif defined ES_SOLARIS
#include <ESTFSolarisConfig.h>
#elif defined ES_DARWIN
#include <ESTFDarwinConfig.h>
#else
#include <config.h>
#endif

#endif /* ! ESTF_CONFIG_H */
