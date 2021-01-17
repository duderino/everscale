#ifndef ES_HTTP_REQUEST_PARSER_H
#include <ESHttpRequestParser.h>
#endif

#ifndef ES_HTTP_RESPONSE_PARSER_H
#include <ESHttpResponseParser.h>
#endif

#ifndef ES_HTTP_REQUEST_FORMATTER_H
#include <ESHttpRequestFormatter.h>
#endif

#ifndef ES_HTTP_RESPONSE_FORMATTER_H
#include <ESHttpResponseFormatter.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_RAND_H
#include <ESBRand.h>
#endif

#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
#endif

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace ES {

static ESB::SimpleFileLogger Logger(stdout, ESB::Logger::Warning);
static ESB::Rand Random(42);
static unsigned char InputBufferStorage[4096];
static unsigned char OutputBufferStorage[4096];
static unsigned char WorkingBufferStorage[4096];
static ESB::Buffer InputBuffer(InputBufferStorage, sizeof(InputBufferStorage));
static ESB::Buffer OutputBuffer(OutputBufferStorage, sizeof(OutputBufferStorage));
static ESB::Buffer WorkingBuffer(WorkingBufferStorage, sizeof(WorkingBufferStorage));
static ESB::DiscardAllocator Allocator(4096, sizeof(ESB::UWord), 1, ESB::SystemAllocator::Instance(), true);
static HttpRequestParser RequestParser(&WorkingBuffer, Allocator);
static HttpResponseParser ResponseParser(&WorkingBuffer, Allocator);
static HttpRequestFormatter RequestFormatter;
static HttpResponseFormatter ResponseFormatter;
static bool ParseRequest(const char *file);
static bool ParseResponse(const char *file);
static bool CompareFiles(int fd1, int fd2);
static bool DumpFile(int fd, const char *filename);

static ESB::SSize FullyRead(int fd, unsigned char *buffer, ESB::UInt32 bytesToRead) {
  ESB::UInt32 bytesRead = 0U;

  while (bytesRead < bytesToRead) {
    ESB::SSize result = read(fd, buffer + bytesRead, bytesToRead - bytesRead);
    if (0 > result) {
      return result;
    }
    if (0 == result) {
      break;
    }
    bytesRead += result;
  }

  return bytesRead;
}

static ESB::SSize FullyWrite(int fd, const unsigned char *buffer, ESB::UInt32 bytesToWrite) {
  ESB::UInt32 bytesWritten = 0U;

  while (bytesWritten < bytesToWrite) {
    ESB::SSize result = write(fd, buffer + bytesWritten, bytesToWrite - bytesWritten);
    if (0 > result) {
      return result;
    }
    if (0 == result) {
      break;
    }
    bytesWritten += result;
  }

  return bytesWritten;
}

bool ParseRequest(const char *inputFileName) {
  int inputFd = open(inputFileName, O_RDONLY);

  if (0 > inputFd) {
    ESB_LOG_ERROR_ERRNO(ESB::LastError(), "cannot open %s", inputFileName);
    return false;
  }

  char outputFileName[NAME_MAX + 1];

  snprintf(outputFileName, sizeof(outputFileName) - 1, "output_%s", inputFileName);
  outputFileName[sizeof(outputFileName) - 1] = 0;

  int outputFd = open(outputFileName, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);

  if (0 > outputFd) {
    ESB_LOG_ERROR_ERRNO(ESB::LastError(), "cannot open %s", inputFileName);
    return false;
  }

  char validationFileName[NAME_MAX + 1];

  snprintf(validationFileName, sizeof(validationFileName) - 1, "validation_%s", inputFileName);
  validationFileName[sizeof(validationFileName) - 1] = 0;

  int validationFd = open(validationFileName, O_RDONLY);

  if (0 > validationFd) {
    ESB_LOG_ERROR_ERRNO(ESB::LastError(), "cannot open %s", inputFileName);
    return false;
  }

  ssize_t result;
  ESB::Error error;
  HttpRequest request;
  HttpHeader *header = 0;

  //
  //  Parse Request Headers
  //

  while (true) {
    error = RequestParser.parseHeaders(&InputBuffer, request);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("need more data from file %s", inputFileName);

      if (false == InputBuffer.isWritable()) {
        ESB_LOG_DEBUG("compacting input buffer for %s", inputFileName);

        if (false == InputBuffer.compact()) {
          ESB_LOG_ERROR("cannot parse %s: parser jammed\n", inputFileName);
          return false;
        }
      }

      result = FullyRead(inputFd, InputBuffer.buffer() + InputBuffer.writePosition(),
                         Random.generate(1, InputBuffer.writable()));

      if (0 > result) {
        ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error reading %s", inputFileName);
        return false;
      }

      if (0 == result) {
        ESB_LOG_ERROR("premature EOF! %s", inputFileName);
        return false;
      }

      InputBuffer.setWritePosition(InputBuffer.writePosition() + result);

      ESB_LOG_DEBUG("read %ld bytes from file %s", (long int)result, inputFileName);

      continue;
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "error parsing %s", inputFileName);
      return false;
    }

    if (ESB_DEBUG_LOGGABLE) {
      ESB_LOG_DEBUG("headers parsed for file %s", inputFileName);

      ESB_LOG_DEBUG("Method: %s", request.method());
      ESB_LOG_DEBUG("RequestUri");

      switch (request.requestUri().type()) {
        case HttpRequestUri::ES_URI_ASTERISK:

          ESB_LOG_DEBUG("  Asterisk");
          break;

        case HttpRequestUri::ES_URI_HTTP:
        case HttpRequestUri::ES_URI_HTTPS:

          ESB_LOG_DEBUG("  Scheme: %s", HttpRequestUri::ES_URI_HTTP == request.requestUri().type() ? "http" : "https");
          ESB_LOG_DEBUG("  Host: %s",
                        0 == request.requestUri().host() ? "none" : (const char *)request.requestUri().host());
          ESB_LOG_DEBUG("  Port: %d", request.requestUri().port());
          ESB_LOG_DEBUG("  AbsPath: %s", request.requestUri().absPath());
          ESB_LOG_DEBUG("  Query: %s",
                        0 == request.requestUri().query() ? "none" : (const char *)request.requestUri().query());
          ESB_LOG_DEBUG("  Fragment: %s",
                        0 == request.requestUri().fragment() ? "none" : (const char *)request.requestUri().fragment());

          break;

        case HttpRequestUri::ES_URI_OTHER:

          ESB_LOG_DEBUG("  Other: %s", request.requestUri().other());

          break;
      }

      ESB_LOG_DEBUG("Version: HTTP/%d.%d", request.httpVersion() / 100, request.httpVersion() % 100 / 10);

      ESB_LOG_DEBUG("Headers");

      for (header = (HttpHeader *)request.headers().first(); header; header = (HttpHeader *)header->next()) {
        ESB_LOG_DEBUG("   %s: %s\n", (const char *)header->fieldName(),
                      0 == header->fieldValue() ? "null" : (const char *)header->fieldValue());
      }
    }

    break;
  }

  //
  // Format Request Headers
  //

  while (true) {
    error = RequestFormatter.formatHeaders(&OutputBuffer, request);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("flushing output buffer for %s", outputFileName);

      if (false == OutputBuffer.isReadable()) {
        ESB_LOG_ERROR("cannot format %s: formatter jammed", outputFileName);
        return false;
      }

      while (OutputBuffer.isReadable()) {
        result = FullyWrite(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

        if (0 > result) {
          ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error writing %s", outputFileName);
          return false;
        }

        OutputBuffer.setReadPosition(result);
        OutputBuffer.compact();
      }

      continue;
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "error formatting %s", outputFileName);
      return false;
    }

    break;
  }

  // flush any header data left in the output buffer

  ESB_LOG_DEBUG("final flushing output buffer for %s", outputFileName);

  while (OutputBuffer.isReadable()) {
    result = FullyWrite(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

    if (0 > result) {
      ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error writing %s", outputFileName);
      return false;
    }

    OutputBuffer.setReadPosition(result);
    OutputBuffer.compact();
  }

  //
  // Simultaneously parse & format request body - each flush becomes a chunk
  //

  ESB::UInt32 bufferOffset = 0;
  ESB::UInt32 chunkSize = 0;
  ESB::UInt32 availableSize = 0;
  ESB::UInt32 bytesWritten = 0;

  while (true) {
    error = RequestParser.parseBody(&InputBuffer, &bufferOffset, &chunkSize);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("need more data from file %s", inputFileName);

      if (false == InputBuffer.isWritable()) {
        ESB_LOG_DEBUG("compacting input buffer for %s", inputFileName);

        if (false == InputBuffer.compact()) {
          ESB_LOG_ERROR("cannot parse %s: parser jammed", inputFileName);
          return false;
        }
      }

      result = FullyRead(inputFd, InputBuffer.buffer() + InputBuffer.writePosition(),
                         Random.generate(1, InputBuffer.writable()));

      if (0 > result) {
        ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error reading %s", inputFileName);
        return false;
      }

      if (0 == result) {
        ESB_LOG_DEBUG(" EOF reading body from: %s", inputFileName);
        break;
      }

      InputBuffer.setWritePosition(InputBuffer.writePosition() + result);

      ESB_LOG_DEBUG("read %ld bytes from file %s", (long int)result, inputFileName);

      continue;
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "error parsing %s", inputFileName);
      return false;
    }

    if (0 == chunkSize) {
      ESB_LOG_DEBUG("\nfinished reading body %s\n", inputFileName);
      break;
    }

    if (ESB_DEBUG_LOGGABLE) {
      char buffer[sizeof(InputBufferStorage) + 1];

      memcpy(buffer, InputBuffer.buffer() + bufferOffset, chunkSize);
      buffer[chunkSize] = 0;

      ESB_LOG_DEBUG("read chunk:");
      ESB_LOG_DEBUG("%s", buffer);
    }

    bytesWritten = 0;

    while (bytesWritten < chunkSize) {
      error = RequestFormatter.beginBlock(&OutputBuffer, chunkSize - bytesWritten, &availableSize);

      if (ESB_AGAIN == error) {
        ESB_LOG_DEBUG("flushing output buffer for %s\n", outputFileName);

        if (false == OutputBuffer.isReadable()) {
          ESB_LOG_ERROR("cannot format %s: formatter jammed", outputFileName);
          return false;
        }

        while (OutputBuffer.isReadable()) {
          result = FullyWrite(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

          if (0 > result) {
            ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error writing %s", outputFileName);
            return false;
          }

          OutputBuffer.setReadPosition(result);
          OutputBuffer.compact();
        }

        continue;
      }

      if (ESB_SUCCESS != error) {
        ESB_LOG_ERROR_ERRNO(error, "error formatting %s", outputFileName);
        return false;
      }

      memcpy(OutputBuffer.buffer() + OutputBuffer.writePosition(), InputBuffer.buffer() + bufferOffset + bytesWritten,
             availableSize);

      OutputBuffer.setWritePosition(OutputBuffer.writePosition() + availableSize);

      bytesWritten += availableSize;

      while (true) {
        error = RequestFormatter.endBlock(&OutputBuffer);

        if (ESB_AGAIN == error) {
          ESB_LOG_DEBUG("flushing output buffer for %s", outputFileName);

          if (false == OutputBuffer.isReadable()) {
            ESB_LOG_ERROR("cannot format %s: formatter jammed", outputFileName);
            return false;
          }

          while (OutputBuffer.isReadable()) {
            result = FullyWrite(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

            if (0 > result) {
              ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error writing %s", outputFileName);
              return false;
            }

            OutputBuffer.setReadPosition(result);
            OutputBuffer.compact();
          }

          continue;
        }

        if (ESB_SUCCESS != error) {
          ESB_LOG_ERROR_ERRNO(error, "error formatting %s", outputFileName);
          return false;
        }

        break;
      }
    }

    RequestParser.consumeBody(&InputBuffer, chunkSize);
  }

  // format last chunk

  while (true) {
    error = RequestFormatter.endBody(&OutputBuffer);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("flushing output buffer for %s", outputFileName);

      if (false == OutputBuffer.isReadable()) {
        ESB_LOG_ERROR("cannot format %s: formatter jammed", outputFileName);
        return false;
      }

      while (OutputBuffer.isReadable()) {
        result = FullyWrite(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

        if (0 > result) {
          ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error writing %s", outputFileName);
          return false;
        }

        OutputBuffer.setReadPosition(result);
        OutputBuffer.compact();
      }

      continue;
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "error formatting %s", outputFileName);
      return false;
    }

    break;
  }

  // flush anything left in the output buffer

  ESB_LOG_DEBUG("final flushing output buffer for %s", outputFileName);

  while (OutputBuffer.isReadable()) {
    result = FullyWrite(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

    if (0 > result) {
      ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error writing %s", outputFileName);
      return false;
    }

    OutputBuffer.setReadPosition(result);
    OutputBuffer.compact();
  }

  // skip trailer if present (99% chance it's not).  only necessary for socket
  // reuse

  while (true) {
    error = RequestParser.skipTrailer(&InputBuffer);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("need more data from file %s", inputFileName);

      if (false == InputBuffer.isWritable()) {
        ESB_LOG_DEBUG("compacting input buffer for %s", inputFileName);

        if (false == InputBuffer.compact()) {
          ESB_LOG_ERROR("cannot parse %s: parser jammed\n", inputFileName);
          return false;
        }
      }

      result = FullyRead(inputFd, InputBuffer.buffer() + InputBuffer.writePosition(),
                         Random.generate(1, InputBuffer.writable()));

      if (0 > result) {
        ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error reading %s", inputFileName);
        return false;
      }

      if (0 == result) {
        ESB_LOG_ERROR("premature EOF! %s", inputFileName);
        return false;
      }

      InputBuffer.setWritePosition(InputBuffer.writePosition() + result);

      ESB_LOG_DEBUG("read %ld bytes from file %s", (long int)result, inputFileName);

      continue;
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "error parsing %s", inputFileName);
      return false;
    }

    ESB_LOG_DEBUG("trailer skipped for file %s", inputFileName);
    break;
  }

  if (!CompareFiles(outputFd, validationFd)) {
    ESB_LOG_ERROR("%s does not match %s", outputFileName, validationFileName);
    DumpFile(validationFd, validationFileName);
    DumpFile(outputFd, outputFileName);
    return false;
  }

  return true;
}

bool ParseResponse(const char *inputFileName) {
  int inputFd = open(inputFileName, O_RDONLY);

  if (0 > inputFd) {
    ESB_LOG_ERROR_ERRNO(ESB::LastError(), "cannot open %s", inputFileName);
    return false;
  }

  char outputFileName[NAME_MAX + 1];

  snprintf(outputFileName, sizeof(outputFileName) - 1, "output_%s", inputFileName);
  outputFileName[sizeof(outputFileName) - 1] = 0;

  int outputFd = open(outputFileName, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);

  if (0 > outputFd) {
    ESB_LOG_ERROR_ERRNO(ESB::LastError(), "cannot open %s", outputFileName);
    return false;
  }

  char validationFileName[NAME_MAX + 1];

  snprintf(validationFileName, sizeof(validationFileName) - 1, "validation_%s", inputFileName);
  validationFileName[sizeof(validationFileName) - 1] = 0;

  int validationFd = open(validationFileName, O_RDONLY);

  if (0 > validationFd) {
    ESB_LOG_ERROR_ERRNO(ESB::LastError(), "cannot open %s", validationFileName);
    return false;
  }

  ssize_t result;
  ESB::Error error;
  HttpResponse response;
  HttpHeader *header = 0;

  //
  //  Parse response headers
  //

  while (true) {
    error = ResponseParser.parseHeaders(&InputBuffer, response);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("need more data from file %s", inputFileName);

      if (false == InputBuffer.isWritable()) {
        ESB_LOG_DEBUG("compacting input buffer for %s", inputFileName);

        if (false == InputBuffer.compact()) {
          ESB_LOG_ERROR("cannot parse %s: parser jammed", inputFileName);
          return false;
        }
      }

      result = FullyRead(inputFd, InputBuffer.buffer() + InputBuffer.writePosition(),
                         Random.generate(1, InputBuffer.writable()));

      if (0 > result) {
        ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error reading %s", inputFileName);
        return false;
      }

      if (0 == result) {
        ESB_LOG_ERROR("premature EOF! %s", inputFileName);
        return false;
      }

      InputBuffer.setWritePosition(InputBuffer.writePosition() + result);

      ESB_LOG_DEBUG("read %ld bytes from file %s", (long int)result, inputFileName);
      continue;
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "error parsing %s", inputFileName);
      return false;
    }

    if (ESB_DEBUG_LOGGABLE) {
      ESB_LOG_DEBUG("headers parsed for file %s", inputFileName);

      ESB_LOG_DEBUG("StatusCode: %d", response.statusCode());
      ESB_LOG_DEBUG("ReasonPhrase: %s", response.reasonPhrase());
      ESB_LOG_DEBUG("Version: HTTP/%d.%d", response.httpVersion() / 100, response.httpVersion() % 100 / 10);

      ESB_LOG_DEBUG("Headers");

      for (header = (HttpHeader *)response.headers().first(); header; header = (HttpHeader *)header->next()) {
        ESB_LOG_DEBUG("   %s: %s", (const char *)header->fieldName(),
                      0 == header->fieldValue() ? "null" : (const char *)header->fieldValue());
      }
    }

    break;
  }

  //
  // Format Response Headers
  //

  while (true) {
    error = ResponseFormatter.formatHeaders(&OutputBuffer, response);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("flushing output buffer for %s", outputFileName);

      if (false == OutputBuffer.isReadable()) {
        ESB_LOG_ERROR("cannot format %s: formatter jammed", outputFileName);
        return false;
      }

      while (OutputBuffer.isReadable()) {
        result = FullyWrite(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

        if (0 > result) {
          ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error writing %s", outputFileName);
          return false;
        }

        OutputBuffer.setReadPosition(result);
        OutputBuffer.compact();
      }

      continue;
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "error formatting %s", outputFileName);
      return false;
    }

    break;
  }

  // flush any header data left in the output buffer

  ESB_LOG_DEBUG("final flushing output buffer for %s", outputFileName);

  while (OutputBuffer.isReadable()) {
    result = FullyWrite(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

    if (0 > result) {
      ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error writing %s", outputFileName);
      return false;
    }

    OutputBuffer.setReadPosition(result);
    OutputBuffer.compact();
  }

  //
  // Simultaneously parse & format response body - each flush becomes a chunk
  //

  ESB::UInt32 bufferOffset = 0;
  ESB::UInt32 chunkSize = 0;
  ESB::UInt32 availableSize = 0;
  ESB::UInt32 bytesWritten = 0;

  while (true) {
    error = ResponseParser.parseBody(&InputBuffer, &bufferOffset, &chunkSize);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("need more data from file %s", inputFileName);

      if (false == InputBuffer.isWritable()) {
        ESB_LOG_DEBUG("compacting input buffer for %s", inputFileName);

        if (false == InputBuffer.compact()) {
          ESB_LOG_ERROR("cannot parse %s: parser jammed", inputFileName);
          return false;
        }
      }

      result = FullyRead(inputFd, InputBuffer.buffer() + InputBuffer.writePosition(),
                         Random.generate(1, InputBuffer.writable()));

      if (0 > result) {
        ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error reading %s", inputFileName);
        return false;
      }

      if (0 == result) {
        ESB_LOG_DEBUG("EOF reading %s", inputFileName);
        break;
      }

      InputBuffer.setWritePosition(InputBuffer.writePosition() + result);

      ESB_LOG_DEBUG("read %ld bytes from file %s", (long int)result, inputFileName);
      continue;
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "error parsing %s", inputFileName);
      return false;
    }

    if (0 == chunkSize) {
      ESB_LOG_DEBUG("finished reading body %s", inputFileName);
      break;
    }

    if (ESB_DEBUG_LOGGABLE) {
      char buffer[sizeof(InputBufferStorage) + 1];

      memcpy(buffer, InputBuffer.buffer() + bufferOffset, chunkSize);
      buffer[chunkSize] = 0;

      ESB_LOG_DEBUG("read chunk:");
      ESB_LOG_DEBUG("%s", buffer);
    }

    bytesWritten = 0;

    while (bytesWritten < chunkSize) {
      error = ResponseFormatter.beginBlock(&OutputBuffer, chunkSize - bytesWritten, &availableSize);

      if (ESB_AGAIN == error) {
        ESB_LOG_DEBUG("flushing output buffer for %s", outputFileName);

        if (false == OutputBuffer.isReadable()) {
          ESB_LOG_ERROR("cannot format %s: formatter jammed", outputFileName);
          return false;
        }

        while (OutputBuffer.isReadable()) {
          result = FullyWrite(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

          if (0 > result) {
            ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error writing %s", outputFileName);
            return false;
          }

          OutputBuffer.setReadPosition(result);
          OutputBuffer.compact();
        }

        continue;
      }

      if (ESB_SUCCESS != error) {
        ESB_LOG_ERROR_ERRNO(error, "error formatting %s", outputFileName);
        return false;
      }

      memcpy(OutputBuffer.buffer() + OutputBuffer.writePosition(), InputBuffer.buffer() + bufferOffset + bytesWritten,
             availableSize);
      OutputBuffer.setWritePosition(OutputBuffer.writePosition() + availableSize);
      bytesWritten += availableSize;

      while (true) {
        error = ResponseFormatter.endBlock(&OutputBuffer);

        if (ESB_AGAIN == error) {
          ESB_LOG_DEBUG("flushing output buffer for %s", outputFileName);

          if (false == OutputBuffer.isReadable()) {
            ESB_LOG_ERROR("cannot format %s: formatter jammed", outputFileName);
            return false;
          }

          while (OutputBuffer.isReadable()) {
            result = FullyWrite(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

            if (0 > result) {
              ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error writing %s", outputFileName);
              return false;
            }

            OutputBuffer.setReadPosition(result);
            OutputBuffer.compact();
          }

          continue;
        }

        if (ESB_SUCCESS != error) {
          ESB_LOG_ERROR_ERRNO(error, "error formatting %s", outputFileName);
          return false;
        }

        break;
      }
    }

    ResponseParser.consumeBody(&InputBuffer, chunkSize);
  }

  // format last chunk

  while (true) {
    error = ResponseFormatter.endBody(&OutputBuffer);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("flushing output buffer for %s", outputFileName);

      if (false == OutputBuffer.isReadable()) {
        ESB_LOG_ERROR("cannot format %s: formatter jammed", outputFileName);
        return false;
      }

      while (OutputBuffer.isReadable()) {
        result = FullyWrite(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

        if (0 > result) {
          ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error writing %s", outputFileName);
          return false;
        }

        OutputBuffer.setReadPosition(result);
        OutputBuffer.compact();
      }

      continue;
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "error formatting %s", outputFileName);
      return false;
    }

    break;
  }

  // flush anything left in the output buffer

  ESB_LOG_DEBUG("final flushing output buffer for %s", outputFileName);

  while (OutputBuffer.isReadable()) {
    result = FullyWrite(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

    if (0 > result) {
      ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error writing %s", outputFileName);
      return false;
    }

    OutputBuffer.setReadPosition(result);
    OutputBuffer.compact();
  }

  // skip trailer if present (99% chance it's not).  only necessary for socket
  // reuse

  while (true) {
    error = ResponseParser.skipTrailer(&InputBuffer);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("need more data from file %s", inputFileName);

      if (false == InputBuffer.isWritable()) {
        ESB_LOG_DEBUG("compacting input buffer for %s", inputFileName);

        if (false == InputBuffer.compact()) {
          ESB_LOG_ERROR("cannot parse %s: parser jammed\n", inputFileName);
          return false;
        }
      }

      result = FullyRead(inputFd, InputBuffer.buffer() + InputBuffer.writePosition(),
                         Random.generate(1, InputBuffer.writable()));

      if (0 > result) {
        ESB_LOG_ERROR_ERRNO(ESB::LastError(), "error reading %s", inputFileName);
        return false;
      }

      if (0 == result) {
        ESB_LOG_ERROR("premature EOF! %s", inputFileName);
        return false;
      }

      InputBuffer.setWritePosition(InputBuffer.writePosition() + result);

      ESB_LOG_DEBUG("read %ld bytes from file %s", (long int)result, inputFileName);

      continue;
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "error parsing %s", inputFileName);
      return false;
    }

    ESB_LOG_DEBUG("trailer skipped for file %s", inputFileName);
    break;
  }

  if (!CompareFiles(outputFd, validationFd)) {
    ESB_LOG_ERROR("%s does not match %s", outputFileName, validationFileName);
    DumpFile(validationFd, validationFileName);
    DumpFile(outputFd, outputFileName);
    return false;
  }

  ESB_LOG_DEBUG("Success!");

  return true;
}

bool DumpFile(int fd, const char *filename) {
  ESB_LOG_DEBUG("FILE %s", filename);

  if (0 > lseek(fd, 0, SEEK_SET)) {
    ESB_LOG_ERROR_ERRNO(ESB::LastError(), "cannot seek on %s", filename);
    return false;
  }

  unsigned char buffer[4096];
  ssize_t result;

  while (true) {
    result = FullyRead(fd, buffer, sizeof(buffer));

    if (0 > result) {
      ESB_LOG_ERROR_ERRNO(ESB::LastError(), "cannot read from %s", filename);
      return false;
    }

    if (0 == result) {
      return true;
    }

    ssize_t result2 = 0;
    int bytesWritten = 0;

    while (bytesWritten < result) {
      result2 = FullyWrite(2, buffer + bytesWritten, result - bytesWritten);
      if (0 >= result2) {
        ESB_LOG_ERROR_ERRNO(ESB::LastError(), "cannot dump to %s", filename);
        return false;
      }
      bytesWritten += result2;
    }

    return true;
  }
}

bool CompareFiles(int fd1, int fd2) {
  if (0 > lseek(fd1, 0, SEEK_SET)) {
    ESB_LOG_ERROR_ERRNO(ESB::LastError(), "cannot seek");
    return false;
  }

  if (0 > lseek(fd2, 0, SEEK_SET)) {
    ESB_LOG_ERROR_ERRNO(ESB::LastError(), "cannot seek");
    return false;
  }

  unsigned char buffer1[4096];
  unsigned char buffer2[4096];
  ssize_t result1;
  ssize_t result2;

  while (true) {
    result1 = FullyRead(fd1, buffer1, sizeof(buffer1));

    if (0 > result1) {
      ESB_LOG_ERROR_ERRNO(ESB::LastError(), "cannot read");
      return false;
    }

    if (0 == result1) {
      result1 = read(fd2, buffer2, sizeof(buffer2));

      if (0 > result1) {
        ESB_LOG_ERROR_ERRNO(ESB::LastError(), "cannot read");
        return false;
      }

      return 0 == result1;
    }

    result2 = FullyRead(fd2, buffer2, result1);

    if (0 > result2) {
      ESB_LOG_ERROR_ERRNO(ESB::LastError(), "cannot read");
      return false;
    }

    if (0 == result2) {
      return false;
    }

    if (result1 != result2) {
      return false;
    }

    for (int i = 0; i < result2; ++i) {
      if (buffer1[i] != buffer2[i]) {
        return false;
      }
    }
  }
}

}  // namespace ES

int main(int argc, char **argv) {
  ESB::Logger::SetInstance(&ES::Logger);

  char currentWorkingDirectory[NAME_MAX + 1];

  if (0 == getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory))) {
    perror("Cannot get cwd");
    return 2;
  }

  DIR *directory = opendir(currentWorkingDirectory);

  for (struct dirent *entry = readdir(directory); entry; entry = readdir(directory)) {
    ES::InputBuffer.clear();
    ES::WorkingBuffer.clear();
    ES::OutputBuffer.clear();
    ES::Allocator.reset();
    ES::RequestParser.reset();
    ES::ResponseParser.reset();
    ES::RequestFormatter.reset();
    ES::ResponseFormatter.reset();

    if (0 == strncmp(entry->d_name, "request", sizeof("request") - 1)) {
      if (false == ES::ParseRequest(entry->d_name)) {
        return 3;
      }

      continue;
    }

    if (0 == strncmp(entry->d_name, "response", sizeof("response") - 1)) {
      if (false == ES::ParseResponse(entry->d_name)) {
        return 4;
      }

      continue;
    }
  }

  closedir(directory);
  directory = 0;

  return 0;
}
