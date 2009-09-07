/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_HEADER_H
#include <AWSHttpHeader.h>
#endif

AWSHttpHeader::AWSHttpHeader(const unsigned char *fieldName, const unsigned char *fieldValue) :
    ESFEmbeddedListElement(),
    _fieldName(fieldName),
    _fieldValue(fieldValue)
{
}

AWSHttpHeader::~AWSHttpHeader()
{
}

ESFCleanupHandler *AWSHttpHeader::getCleanupHandler()
{
    return 0;
}


