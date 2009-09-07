/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_MESSAGE_H
#include <AWSHttpMessage.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

AWSHttpMessage::AWSHttpMessage() :
    _flags(0x00),
    _version(110),
    _headers()
{
}

AWSHttpMessage::~AWSHttpMessage()
{
}

const AWSHttpHeader *AWSHttpMessage::getHeader(unsigned const char *fieldName) const
{
    if (0 == fieldName)
    {
        return 0;
    }

    for (const AWSHttpHeader *header = (const AWSHttpHeader *) _headers.getFirst();
         header;
         header = (const AWSHttpHeader *) header->getNext())
    {
        if (0 == strcasecmp((const char *) fieldName, (const char *) header->getFieldName()))
        {
            return header;
        }
    }

    return 0;
}


ESFError AWSHttpMessage::addHeader(unsigned const char *fieldName,
                                   unsigned const char *fieldValue,
                                   ESFAllocator *allocator)
{
    if (! fieldName || ! allocator)
    {
        return ESF_NULL_POINTER;
    }

    int fieldNameLength = strlen((const char *) fieldName);
    int fieldValueLength = strlen((const char *) fieldValue);

    unsigned char *block = (unsigned char *) allocator->allocate(sizeof(AWSHttpHeader) +
                                                                 fieldNameLength +
                                                                 fieldValueLength + 2);

    if (! block)
    {
        return ESF_OUT_OF_MEMORY;
    }

    memcpy(block + sizeof(AWSHttpHeader), fieldName, fieldNameLength);
    block[sizeof(AWSHttpHeader) + fieldNameLength] = 0;

    memcpy(block + sizeof(AWSHttpHeader) + fieldNameLength + 1, fieldValue, fieldValueLength);
    block[sizeof(AWSHttpHeader) + fieldNameLength + fieldValueLength + 1] = 0;

    AWSHttpHeader *header = new(block) AWSHttpHeader(block + sizeof(AWSHttpHeader),
                                                     block + sizeof(AWSHttpHeader) + fieldNameLength + 1);

    _headers.addLast(header);

    return ESF_SUCCESS;
}

ESFError AWSHttpMessage::addHeader(ESFAllocator *allocator,
                                   unsigned const char *fieldName,
                                   unsigned const char *fieldValueFormat,
                                   ...)
{
    char buffer[1024];

    va_list vaList;

    va_start(vaList, (const char *) fieldValueFormat);

    vsnprintf(buffer, sizeof(buffer), (const char *) fieldValueFormat, vaList);

    va_end( vaList );

    return addHeader(fieldName, (unsigned const char *) buffer, allocator);
}
