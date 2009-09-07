/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_RESPONSE_PARSER_H
#define AWS_HTTP_RESPONSE_PARSER_H

#ifndef AWS_HTTP_RESPONSE_H
#include <AWSHttpResponse.h>
#endif

#ifndef AWS_HTTP_MESSAGE_PARSER_H
#include <AWSHttpMessageParser.h>
#endif

/**
 * Parses a HTTP Response as defined in RFC 2616 and RFC 2396
 */
class AWSHttpResponseParser : public AWSHttpMessageParser
{
public:

    /** Create a new response parser
     *
     * @param workingBuffer Temporary storage for parsing
     * @param allocator The discard allocator to use for allocating internal strings.
     */
    AWSHttpResponseParser(ESFBuffer *workingBuffer, ESFDiscardAllocator *allocator);

    virtual ~AWSHttpResponseParser();

    /**
     * Reset the parser
     */
    virtual void reset();

protected:

    // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
    virtual ESFError parseStartLine(ESFBuffer *inputBuffer, AWSHttpMessage *message);

    virtual bool isBodyNotAllowed(AWSHttpMessage *message);

private:

    // Disabled
    AWSHttpResponseParser(const AWSHttpResponseParser &parser);
    void operator=(const AWSHttpResponseParser &parser);

    // Status-Code    = 3DIGIT
    ESFError parseStatusCode(ESFBuffer *inputBuffer, AWSHttpResponse *response);

    // Reason-Phrase  = *<TEXT, excluding CR, LF>
    ESFError parseReasonPhrase(ESFBuffer *inputBuffer, AWSHttpResponse *response);

    int _responseState;
};

#endif
