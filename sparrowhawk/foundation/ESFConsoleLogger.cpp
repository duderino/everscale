/** @file ESFConsoleLogger.cpp
 *  @brief An implementation of the ESFLogger interface that logs messages to
 *  the system's console.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_CONSOLE_LOGGER_H
#include <ESFConsoleLogger.h>
#endif

#ifdef ALLOW_CONSOLE_LOGGING

#ifndef ESF_THREAD_H
#include <ESFThread.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#else
#error "Need stdio.h or equivalent"
#endif

#if defined HAVE_STDARG_H
#include <stdarg.h>
#else
#error "Need stdarg.h or equivalent"
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#error "Need string.h or equivalent"
#endif

#ifndef ESF_CONSOLE_LOGGER_BUFFER_SIZE
#define ESF_CONSOLE_LOGGER_BUFFER_SIZE 256
#endif

static const char *EmergencyString = "EMERGENCY";
static const char *AlertString = "ALERT";
static const char *CriticalString = "CRITICAL";
static const char *ErrorString = "ERROR";
static const char *WarningString = "WARNING";
static const char *NoticeString = "NOTICE";
static const char *InfoString = "INFO";
static const char *DebugString = "DEBUG";

static const char *CodeToString(ESFLogger::Severity severity) {
    switch (severity) {
    case ESFLogger::Emergency:
        return EmergencyString;
    case ESFLogger::Alert:
        return AlertString;
    case ESFLogger::Critical:
        return CriticalString;
    case ESFLogger::Error:
        return ErrorString;
    case ESFLogger::Warning:
        return WarningString;
    case ESFLogger::Notice:
        return NoticeString;
    case ESFLogger::Info:
        return InfoString;
    case ESFLogger::Debug:
        return DebugString;
    default:
        return "";
    }
}

#endif /* defined ALLOW_CONSOLE_LOGGING */

ESFConsoleLogger ESFConsoleLogger::_Instance;

ESFError ESFConsoleLogger::Initialize(Severity severity) {
    _Instance._severity = severity;

#ifdef ALLOW_CONSOLE_LOGGING
    return ESF_SUCCESS;
#else
    return ESF_OPERATION_NOT_SUPPORTED;
#endif
}

ESFError ESFConsoleLogger::Destroy() {
#ifdef ALLOW_CONSOLE_LOGGING
    return ESF_SUCCESS;
#else
    return ESF_OPERATION_NOT_SUPPORTED;
#endif
}

ESFConsoleLogger *
ESFConsoleLogger::Instance() {
    return &_Instance;
}

ESFConsoleLogger::ESFConsoleLogger() :
    _severity(None) {
}

ESFConsoleLogger::~ESFConsoleLogger() {
}

bool ESFConsoleLogger::isLoggable(Severity severity) {
    return severity > _severity ? false : true;
}

void ESFConsoleLogger::setSeverity(Severity severity) {
    _severity = severity;
}

ESFError ESFConsoleLogger::log(Severity severity, const char *file, int line, const char *format, ...) {
#ifdef ALLOW_CONSOLE_LOGGING
    if (!file || !format) {
        return ESF_NULL_POINTER;
    }

    if (severity > _severity) {
        return ESF_SUCCESS;
    }

#ifdef HAVE_STRLEN
    int length = strlen(format);
#else
#error "strlen or equivalent is required"
#endif

#ifdef HAVE_VA_LIST_T
    va_list vaList;
#else
#error "va_list or equivalent is required"
#endif

#ifdef HAVE_VA_START
    va_start( vaList, format );
#else
#error "va_start or equivalent is required"
#endif

    //  55 is a guess for the length of the data we prepend to the message.
    if (length + 55 >= ESF_CONSOLE_LOGGER_BUFFER_SIZE) {
#ifdef HAVE_VFPRINTF
        vfprintf(stderr, format, vaList);
#else
#error "vfprintf or equivalent is required"
#endif

#ifdef HAVE_VA_END
        va_end( vaList );
#else
#error "va_end or equivalent is required"
#endif
        fprintf(stderr, "\n");

        return ESF_SUCCESS;
    }

    char buffer[ESF_CONSOLE_LOGGER_BUFFER_SIZE];

#ifdef HAVE_SNPRINTF
    snprintf(buffer, sizeof(buffer), "%s,%s,%d," THREAD_ID_FORMAT " ", CodeToString(severity), file, line,
            ESFThread::GetThreadId());
#else
#error "snprintf or equivalent is required"
#endif

#ifdef HAVE_STRLEN
    if (length + strlen(buffer) >= ESF_CONSOLE_LOGGER_BUFFER_SIZE)
#else
#error "strlen or equivalent is required"
#endif
    {
#ifdef HAVE_VFPRINTF
        vfprintf(stderr, format, vaList);
#else
#error "vfprintf or equivalent is required"
#endif

#ifdef HAVE_VA_END
        va_end( vaList );
#else
#error "va_end or equivalent is required"
#endif

        fprintf(stderr, "\n");

        return ESF_SUCCESS;
    }

#ifdef HAVE_STRCAT
    strcat(buffer, format);
#else
#error "strcat or equivalent is required"
#endif

#ifdef HAVE_VFPRINTF
    vfprintf(stderr, buffer, vaList);
#else
#error "vfprintf or equivalent is required"
#endif

#ifdef HAVE_VA_END
    va_end( vaList );
#else
#error "va_end or equivalent is required"
#endif

    fprintf(stderr, "\n");

    return ESF_SUCCESS;

#else /* ! defined ALLOW_CONSOLE_LOGGING */

    return ESF_OPERATION_NOT_SUPPORTED;

#endif
}

