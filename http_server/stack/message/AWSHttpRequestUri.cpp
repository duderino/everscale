/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_REQUEST_URI_H
#include <AWSHttpRequestUri.h>
#endif

#include <string.h>

AWSHttpRequestUri::AWSHttpRequestUri(UriType type) :
    _type(type),
    _port(-1),
    _username(0),
    _password(0),
    _host(0),
    _absPath(0),
    _query(0),
    _fragment(0),
    _other(0)
{
}

AWSHttpRequestUri::AWSHttpRequestUri() :
    _type(AWS_URI_HTTP),
    _port(-1),
    _username(0),
    _password(0),
    _host(0),
    _absPath(0),
    _query(0),
    _fragment(0),
    _other(0)
{
}

AWSHttpRequestUri::~AWSHttpRequestUri()
{
}

void AWSHttpRequestUri::reset()
{
    _type = AWS_URI_HTTP;
    _port = -1;
    _username = 0;
    _password = 0;
    _host = 0;
    _absPath = 0;
    _query = 0;
    _fragment = 0;
    _other = 0;
}

int AWSHttpRequestUri::Compare(const AWSHttpRequestUri *r1, const AWSHttpRequestUri *r2)
{
    if (AWS_URI_ASTERISK == r1->getType() && AWS_URI_ASTERISK == r2->getType())
    {
        return 0;
    }

    if (AWS_URI_OTHER == r1->getType() && AWS_URI_OTHER == r2->getType())
    {
        return strcmp((const char *) r1->getOther(), (const char *) r2->getOther());
    }

    int result = 0;

    // Comparisons of scheme names MUST be case-insensitive
    if (r1->getType() != r2->getType())
    {
        return AWS_URI_HTTP == r1->getType() ? 1 : -1;
    }

    // todo add username, password, and anchor to comparison

    // Comparisons of host names MUST be case-insensitive
    if (r1->getHost() != r2->getHost())
    {
        if (0 == r1->getHost())
        {
            return -1;
        }

        if (0 == r2->getHost())
        {
            return 1;
        }

        result = strcasecmp((const char *) r1->getHost(), (const char *) r2->getHost());

        if (0 != result)
        {
            return result;
        }
    }

    // A port that is empty or not given is equivalent to the default port for that URI-reference;

    int port1 = 0 <= r1->getPort() ? r1->getPort() :
                                     (AWS_URI_HTTP == r1->getType() ? 80 : 443);
    int port2 = 0 <= r2->getPort() ? r2->getPort() :
                                     (AWS_URI_HTTP == r2->getType() ? 80 : 443);

    if (0 != port1 - port2)
    {
        return port1 - port2;
    }

    // An empty abs_path is equivalent to an abs_path of "/".

    const char *absPath1 = 0 == r1->getAbsPath() ? "/" : (const char *) r1->getAbsPath();
    const char *absPath2 = 0 == r2->getAbsPath() ? "/" : (const char *) r2->getAbsPath();

    if (absPath1 != absPath2)
    {
        result = strcmp(absPath1, absPath2);

        if (0 != result)
        {
            return result;
        }
    }

    if (r1->getQuery() != r2->getQuery())
    {
        if (0 == r1->getQuery())
        {
            return -1;
        }

        if (0 == r2->getQuery())
        {
            return 1;
        }

        result = strcmp((const char *) r1->getQuery(), (const char *) r2->getQuery());

        if (0 != result)
        {
            return result;
        }
    }

    return 0;
}
