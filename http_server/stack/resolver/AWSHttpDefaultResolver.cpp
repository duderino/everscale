/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_DEFAULT_RESOLVER_H
#include <AWSHttpDefaultResolver.h>
#endif

#include <netdb.h>
#include <string.h>

AWSHttpDefaultResolver::AWSHttpDefaultResolver(ESFLogger *logger) :
    AWSHttpResolver(),
    _logger(logger)
{
}

AWSHttpDefaultResolver::~AWSHttpDefaultResolver()
{
}

ESFError AWSHttpDefaultResolver::resolve(const AWSHttpRequest *request,
                                         ESFSocketAddress *address)
{
    if (0 == request || 0 == address)
    {
        return ESF_NULL_POINTER;
    }

    unsigned char hostname[1024];
    hostname[0] = 0;
    ESFUInt16 port = 0;
    bool isSecure = false;

    ESFError error = request->parsePeerAddress(hostname,
                                               sizeof(hostname),
                                               &port,
                                               &isSecure);

    if (ESF_SUCCESS != error)
    {
        if (_logger->isLoggable(ESFLogger::Warning))
        {
            _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                         "[resolver] Cannot extract hostname from request");
        }

        return error;
    }

    /* Linux:
     *
     * Glibc2  also  has  reentrant versions gethostbyname_r() and gethostby-
     * name2_r().  These return 0 on success and nonzero on error. The result
     * of  the  call is now stored in the struct with address ret.  After the
     * call, *result will be NULL on error or point to the result on success.
     * Auxiliary  data is stored in the buffer buf of length buflen.  (If the
     * buffer is too small, these functions will return ERANGE.)   No  global
     * variable  h_errno  is modified, but the address of a variable in which
     * to store error numbers is passed in h_errnop.
     *
     * Stevens:
     *
     * Current implementations of gethostbyname can return up to 35 alias
     * pointers, 35 address pointers, and internally use an 8192-byte buffer to
     * hold the alias names and addresses.  So a buffer size of 8192 bytes
     * should be adequate
     */

    char buffer[8192];
    struct hostent hostEntry;
    int hostErrno = 0;
    struct hostent *result = 0;

    memset(&hostEntry, 0, sizeof(hostEntry));

    if (0 != gethostbyname_r((const char *) hostname,
                             &hostEntry,
                             buffer,
                             sizeof(buffer),
                             &result,
                             &hostErrno))
    {
        switch(hostErrno)
        {
            case HOST_NOT_FOUND:
            case NO_ADDRESS:

                if (_logger->isLoggable(ESFLogger::Warning))
                {
                    _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                                 "[resolver] Cannot resolve hostname %s",
                                 hostname);
                }

                return ESF_CANNOT_FIND;

            case TRY_AGAIN:

                if (_logger->isLoggable(ESFLogger::Error))
                {
                    _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                                 "[resolver] Temporary error resolving hostname %s",
                                 hostname);
                }

                return ESF_AGAIN;

            case NO_RECOVERY:
            default:

                if (_logger->isLoggable(ESFLogger::Error))
                {
                    _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                                 "[resolver] Permanent error resolving hostname %s",
                                 hostname);
                }

                return ESF_OTHER_ERROR;
        }
    }

    memset(address->getAddress(), 0, sizeof(ESFSocketAddress::Address));

    memcpy(&address->getAddress()->sin_addr, hostEntry.h_addr_list[0], sizeof(address->getAddress()->sin_addr));
    address->getAddress()->sin_family = AF_INET;
    address->getAddress()->sin_port = htons(port);

    address->setTransport(isSecure ? ESFSocketAddress::TLS : ESFSocketAddress::TCP);

    return ESF_SUCCESS;
}


