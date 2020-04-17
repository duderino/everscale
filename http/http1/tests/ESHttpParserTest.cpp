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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace ES {

static const int Debug = 9;
static ESB::Rand Random(42);
static unsigned char InputBufferStorage[4096];
static unsigned char OutputBufferStorage[4096];
static unsigned char WorkingBufferStorage[4096];
static ESB::Buffer InputBuffer(InputBufferStorage, sizeof(InputBufferStorage));
static ESB::Buffer OutputBuffer(OutputBufferStorage,
                                sizeof(OutputBufferStorage));
static ESB::Buffer WorkingBuffer(WorkingBufferStorage,
                                 sizeof(WorkingBufferStorage));
static ESB::DiscardAllocator Allocator(4096, sizeof(ESB::UWord), 1,
                                       ESB::SystemAllocator::Instance(), true);
static HttpRequestParser RequestParser(&WorkingBuffer, Allocator);
static HttpResponseParser ResponseParser(&WorkingBuffer, Allocator);
static HttpRequestFormatter RequestFormatter;
static HttpResponseFormatter ResponseFormatter;
static bool ParseRequest(const char *file);
static bool ParseResponse(const char *file);
static bool CompareFiles(int fd1, int fd2);

bool ParseRequest(const char *inputFileName) {
  int inputFd = open(inputFileName, O_RDONLY);

  if (0 > inputFd) {
    fprintf(stderr, "cannot open %s: %s\n", inputFileName, strerror(errno));
    return false;
  }

  char outputFileName[NAME_MAX + 1];

  snprintf(outputFileName, sizeof(outputFileName) - 1, "output_%s",
           inputFileName);
  outputFileName[sizeof(outputFileName) - 1] = 0;

  int outputFd =
      open(outputFileName, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);

  if (0 > outputFd) {
    fprintf(stderr, "cannot open %s: %s\n", outputFileName, strerror(errno));
    return false;
  }

  char validationFileName[NAME_MAX + 1];

  snprintf(validationFileName, sizeof(validationFileName) - 1, "validation_%s",
           inputFileName);
  validationFileName[sizeof(validationFileName) - 1] = 0;

  int validationFd = open(validationFileName, O_RDONLY);

  if (0 > validationFd) {
    fprintf(stderr, "cannot open %s: %s\n", validationFileName,
            strerror(errno));
    return false;
  }

  ssize_t result;
  int bytesToRead;
  ESB::Error error;
  HttpRequest request;
  HttpHeader *header = 0;

  //
  //  Parse Request Headers
  //

  while (true) {
    error = RequestParser.parseHeaders(&InputBuffer, request);

    if (ESB_AGAIN == error) {
      if (1 < Debug)
        fprintf(stderr, "need more data from file %s\n", inputFileName);

      bytesToRead = Random.generate(1, InputBuffer.writable());

      if (false == InputBuffer.isWritable()) {
        if (1 < Debug)
          fprintf(stderr, "compacting input buffer for %s\n", inputFileName);

        if (false == InputBuffer.compact()) {
          fprintf(stderr, "cannot parse %s: parser jammed\n", inputFileName);
          return false;
        }
      }

      result = read(inputFd, InputBuffer.buffer() + InputBuffer.writePosition(),
                    bytesToRead);

      if (0 > result) {
        fprintf(stderr, "error reading %s: %s\n", inputFileName,
                strerror(errno));
        return false;
      }

      if (0 == result) {
        fprintf(stderr, "premature EOF! %s\n", inputFileName);
        return false;
      }

      InputBuffer.setWritePosition(InputBuffer.writePosition() + result);

      if (1 < Debug)
        fprintf(stderr, "read %ld bytes from file %s\n", (long int)result,
                inputFileName);

      continue;
    }

    if (ESB_SUCCESS != error) {
      fprintf(stderr, "error parsing %s: %d\n", inputFileName, error);
      return false;
    }

    if (Debug) {
      fprintf(stderr, "headers parsed for file %s\n", inputFileName);

      fprintf(stderr, "Method: %s\n", request.method());
      fprintf(stderr, "RequestUri\n");

      switch (request.requestUri().type()) {
        case HttpRequestUri::ES_URI_ASTERISK:

          fprintf(stderr, "  Asterisk\n");
          break;

        case HttpRequestUri::ES_URI_HTTP:
        case HttpRequestUri::ES_URI_HTTPS:

          fprintf(stderr, "  Scheme: %s\n",
                  HttpRequestUri::ES_URI_HTTP == request.requestUri().type()
                      ? "http"
                      : "https");
          fprintf(stderr, "  Host: %s\n",
                  0 == request.requestUri().host()
                      ? "none"
                      : (const char *)request.requestUri().host());
          fprintf(stderr, "  Port: %d\n", request.requestUri().port());
          fprintf(stderr, "  AbsPath: %s\n", request.requestUri().absPath());
          fprintf(stderr, "  Query: %s\n",
                  0 == request.requestUri().query()
                      ? "none"
                      : (const char *)request.requestUri().query());
          fprintf(stderr, "  Fragment: %s\n",
                  0 == request.requestUri().fragment()
                      ? "none"
                      : (const char *)request.requestUri().fragment());

          break;

        case HttpRequestUri::ES_URI_OTHER:

          fprintf(stderr, "  Other: %s\n", request.requestUri().other());

          break;
      }

      fprintf(stderr, "Version: HTTP/%d.%d\n", request.httpVersion() / 100,
              request.httpVersion() % 100 / 10);

      fprintf(stderr, "Headers\n");

      for (header = (HttpHeader *)request.headers().first(); header;
           header = (HttpHeader *)header->next()) {
        fprintf(stderr, "   %s: %s\n", (const char *)header->fieldName(),
                0 == header->fieldValue() ? "null"
                                          : (const char *)header->fieldValue());
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
      if (1 < Debug)
        fprintf(stderr, "flushing output buffer for %s\n", outputFileName);

      if (false == OutputBuffer.isReadable()) {
        fprintf(stderr, "cannot format %s: formatter jammed\n", outputFileName);
        return false;
      }

      while (OutputBuffer.isReadable()) {
        result =
            write(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

        if (0 > result) {
          fprintf(stderr, "error writing %s: %s\n", outputFileName,
                  strerror(errno));
          return false;
        }

        OutputBuffer.setReadPosition(result);
        OutputBuffer.compact();
      }

      continue;
    }

    if (ESB_SUCCESS != error) {
      fprintf(stderr, "error formatting %s: %d\n", outputFileName, error);
      return false;
    }

    break;
  }

  // flush any header data left in the output buffer

  if (1 < Debug)
    fprintf(stderr, "final flushing output buffer for %s\n", outputFileName);

  while (OutputBuffer.isReadable()) {
    result = write(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

    if (0 > result) {
      fprintf(stderr, "error writing %s: %s\n", outputFileName,
              strerror(errno));
      return false;
    }

    OutputBuffer.setReadPosition(result);
    OutputBuffer.compact();
  }

  //
  // Simultaneously parse & format request body - each flush becomes a chunk
  //

  int startingPosition = 0;
  int chunkSize = 0;
  int availableSize = 0;
  int bytesWritten = 0;

  while (true) {
    error =
        RequestParser.parseBody(&InputBuffer, &startingPosition, &chunkSize);

    if (ESB_AGAIN == error) {
      if (1 < Debug)
        fprintf(stderr, "need more data from file %s\n", inputFileName);

      bytesToRead = Random.generate(1, InputBuffer.writable());

      if (false == InputBuffer.isWritable()) {
        if (1 < Debug)
          fprintf(stderr, "compacting input buffer for %s\n", inputFileName);

        if (false == InputBuffer.compact()) {
          fprintf(stderr, "cannot parse %s: parser jammed\n", inputFileName);
          return false;
        }
      }

      result = read(inputFd, InputBuffer.buffer() + InputBuffer.writePosition(),
                    bytesToRead);

      if (0 > result) {
        fprintf(stderr, "error reading %s: %s\n", inputFileName,
                strerror(errno));
        return false;
      }

      if (0 == result) {
        fprintf(stderr, "EOF reading body from: %s\n", inputFileName);
        break;
      }

      InputBuffer.setWritePosition(InputBuffer.writePosition() + result);

      if (1 < Debug)
        fprintf(stderr, "read %ld bytes from file %s\n", (long int)result,
                inputFileName);

      continue;
    }

    if (ESB_SUCCESS != error) {
      fprintf(stderr, "error parsing %s: %d\n", inputFileName, error);
      return false;
    }

    if (0 == chunkSize) {
      if (Debug) fprintf(stderr, "\nfinished reading body %s\n", inputFileName);

      break;
    }

    if (Debug) {
      char buffer[sizeof(InputBufferStorage) + 1];

      memcpy(buffer, InputBuffer.buffer() + startingPosition, chunkSize);
      buffer[chunkSize] = 0;

      if (3 < Debug) fprintf(stderr, "read chunk:\n");
      if (Debug) fprintf(stderr, "%s", buffer);
    }

    bytesWritten = 0;

    while (bytesWritten < chunkSize) {
      error = RequestFormatter.beginBlock(
          &OutputBuffer, chunkSize - bytesWritten, &availableSize);

      if (ESB_AGAIN == error) {
        if (1 < Debug)
          fprintf(stderr, "flushing output buffer for %s\n", outputFileName);

        if (false == OutputBuffer.isReadable()) {
          fprintf(stderr, "cannot format %s: formatter jammed\n",
                  outputFileName);
          return false;
        }

        while (OutputBuffer.isReadable()) {
          result =
              write(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

          if (0 > result) {
            fprintf(stderr, "error writing %s: %s\n", outputFileName,
                    strerror(errno));
            return false;
          }

          OutputBuffer.setReadPosition(result);
          OutputBuffer.compact();
        }

        continue;
      }

      if (ESB_SUCCESS != error) {
        fprintf(stderr, "error formatting %s: %d\n", outputFileName, error);
        return false;
      }

      memcpy(OutputBuffer.buffer() + OutputBuffer.writePosition(),
             InputBuffer.buffer() + startingPosition + bytesWritten,
             availableSize);

      OutputBuffer.setWritePosition(OutputBuffer.writePosition() +
                                    availableSize);

      bytesWritten += availableSize;

      while (true) {
        error = RequestFormatter.endBlock(&OutputBuffer);

        if (ESB_AGAIN == error) {
          if (1 < Debug)
            fprintf(stderr, "flushing output buffer for %s\n", outputFileName);

          if (false == OutputBuffer.isReadable()) {
            fprintf(stderr, "cannot format %s: formatter jammed\n",
                    outputFileName);
            return false;
          }

          while (OutputBuffer.isReadable()) {
            result =
                write(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

            if (0 > result) {
              fprintf(stderr, "error writing %s: %s\n", outputFileName,
                      strerror(errno));
              return false;
            }

            OutputBuffer.setReadPosition(result);
            OutputBuffer.compact();
          }

          continue;
        }

        if (ESB_SUCCESS != error) {
          fprintf(stderr, "error formatting %s: %d\n", outputFileName, error);
          return false;
        }

        break;
      }
    }
  }

  // format last chunk

  while (true) {
    error = RequestFormatter.endBody(&OutputBuffer);

    if (ESB_AGAIN == error) {
      if (1 < Debug)
        fprintf(stderr, "flushing output buffer for %s\n", outputFileName);

      if (false == OutputBuffer.isReadable()) {
        fprintf(stderr, "cannot format %s: formatter jammed\n", outputFileName);
        return false;
      }

      while (OutputBuffer.isReadable()) {
        result =
            write(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

        if (0 > result) {
          fprintf(stderr, "error writing %s: %s\n", outputFileName,
                  strerror(errno));
          return false;
        }

        OutputBuffer.setReadPosition(result);
        OutputBuffer.compact();
      }

      continue;
    }

    if (ESB_SUCCESS != error) {
      fprintf(stderr, "error formatting %s: %d\n", outputFileName, error);
      return false;
    }

    break;
  }

  // flush anything left in the output buffer

  if (1 < Debug)
    fprintf(stderr, "final flushing output buffer for %s\n", outputFileName);

  while (OutputBuffer.isReadable()) {
    result = write(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

    if (0 > result) {
      fprintf(stderr, "error writing %s: %s\n", outputFileName,
              strerror(errno));
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
      if (1 < Debug)
        fprintf(stderr, "need more data from file %s\n", inputFileName);

      bytesToRead = Random.generate(1, InputBuffer.writable());

      if (false == InputBuffer.isWritable()) {
        if (1 < Debug)
          fprintf(stderr, "compacting input buffer for %s\n", inputFileName);

        if (false == InputBuffer.compact()) {
          fprintf(stderr, "cannot parse %s: parser jammed\n", inputFileName);
          return false;
        }
      }

      result = read(inputFd, InputBuffer.buffer() + InputBuffer.writePosition(),
                    bytesToRead);

      if (0 > result) {
        fprintf(stderr, "error reading %s: %s\n", inputFileName,
                strerror(errno));
        return false;
      }

      if (0 == result) {
        fprintf(stderr, "premature EOF! %s\n", inputFileName);
        return false;
      }

      InputBuffer.setWritePosition(InputBuffer.writePosition() + result);

      if (1 < Debug)
        fprintf(stderr, "read %ld bytes from file %s\n", (long int)result,
                inputFileName);

      continue;
    }

    if (ESB_SUCCESS != error) {
      fprintf(stderr, "error parsing %s: %d\n", inputFileName, error);
      return false;
    }

    if (Debug) {
      fprintf(stderr, "trailer skipped for file %s\n", inputFileName);
    }

    break;
  }

  if (false == CompareFiles(outputFd, validationFd)) {
    fprintf(stderr, "%s does not match %s\n", outputFileName,
            validationFileName);
    return false;
  }

  return true;
}

bool ParseResponse(const char *inputFileName) {
  int inputFd = open(inputFileName, O_RDONLY);

  if (0 > inputFd) {
    fprintf(stderr, "cannot open %s: %s\n", inputFileName, strerror(errno));
    return false;
  }

  char outputFileName[NAME_MAX + 1];

  snprintf(outputFileName, sizeof(outputFileName) - 1, "output_%s",
           inputFileName);
  outputFileName[sizeof(outputFileName) - 1] = 0;

  int outputFd =
      open(outputFileName, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);

  if (0 > outputFd) {
    fprintf(stderr, "cannot open %s: %s\n", outputFileName, strerror(errno));
    return false;
  }

  char validationFileName[NAME_MAX + 1];

  snprintf(validationFileName, sizeof(validationFileName) - 1, "validation_%s",
           inputFileName);
  validationFileName[sizeof(validationFileName) - 1] = 0;

  int validationFd = open(validationFileName, O_RDONLY);

  if (0 > validationFd) {
    fprintf(stderr, "cannot open %s: %s\n", validationFileName,
            strerror(errno));
    return false;
  }

  ssize_t result;
  int bytesToRead;
  ESB::Error error;
  HttpResponse response;
  HttpHeader *header = 0;

  //
  //  Parse response headers
  //

  while (true) {
    error = ResponseParser.parseHeaders(&InputBuffer, response);

    if (ESB_AGAIN == error) {
      if (1 < Debug)
        fprintf(stderr, "need more data from file %s\n", inputFileName);

      bytesToRead = Random.generate(1, InputBuffer.writable());

      if (false == InputBuffer.isWritable()) {
        if (1 < Debug)
          fprintf(stderr, "compacting input buffer for %s\n", inputFileName);

        if (false == InputBuffer.compact()) {
          fprintf(stderr, "cannot parse %s: parser jammed\n", inputFileName);
          return false;
        }
      }

      result = read(inputFd, InputBuffer.buffer() + InputBuffer.writePosition(),
                    bytesToRead);

      if (0 > result) {
        fprintf(stderr, "error reading %s: %s\n", inputFileName,
                strerror(errno));
        return false;
      }

      if (0 == result) {
        fprintf(stderr, "premature EOF! %s\n", inputFileName);
        return false;
      }

      InputBuffer.setWritePosition(InputBuffer.writePosition() + result);

      if (1 < Debug)
        fprintf(stderr, "read %ld bytes from file %s\n", (long int)result,
                inputFileName);

      continue;
    }

    if (ESB_SUCCESS != error) {
      fprintf(stderr, "error parsing %s: %d\n", inputFileName, error);
      return false;
    }

    if (Debug) {
      fprintf(stderr, "headers parsed for file %s\n", inputFileName);

      fprintf(stderr, "StatusCode: %d\n", response.statusCode());
      fprintf(stderr, "ReasonPhrase: %s\n", response.reasonPhrase());
      fprintf(stderr, "Version: HTTP/%d.%d\n", response.httpVersion() / 100,
              response.httpVersion() % 100 / 10);

      fprintf(stderr, "Headers\n");

      for (header = (HttpHeader *)response.headers().first(); header;
           header = (HttpHeader *)header->next()) {
        fprintf(stderr, "   %s: %s\n", (const char *)header->fieldName(),
                0 == header->fieldValue() ? "null"
                                          : (const char *)header->fieldValue());
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
      if (1 < Debug)
        fprintf(stderr, "flushing output buffer for %s\n", outputFileName);

      if (false == OutputBuffer.isReadable()) {
        fprintf(stderr, "cannot format %s: formatter jammed\n", outputFileName);
        return false;
      }

      while (OutputBuffer.isReadable()) {
        result =
            write(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

        if (0 > result) {
          fprintf(stderr, "error writing %s: %s\n", outputFileName,
                  strerror(errno));
          return false;
        }

        OutputBuffer.setReadPosition(result);
        OutputBuffer.compact();
      }

      continue;
    }

    if (ESB_SUCCESS != error) {
      fprintf(stderr, "error formatting %s: %d\n", outputFileName, error);
      return false;
    }

    break;
  }

  // flush any header data left in the output buffer

  if (1 < Debug)
    fprintf(stderr, "final flushing output buffer for %s\n", outputFileName);

  while (OutputBuffer.isReadable()) {
    result = write(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

    if (0 > result) {
      fprintf(stderr, "error writing %s: %s\n", outputFileName,
              strerror(errno));
      return false;
    }

    OutputBuffer.setReadPosition(result);
    OutputBuffer.compact();
  }

  //
  // Simultaneously parse & format response body - each flush becomes a chunk
  //

  int startingPosition = 0;
  int chunkSize = 0;
  int availableSize = 0;
  int bytesWritten = 0;

  while (true) {
    error =
        ResponseParser.parseBody(&InputBuffer, &startingPosition, &chunkSize);

    if (ESB_AGAIN == error) {
      if (1 < Debug)
        fprintf(stderr, "need more data from file %s\n", inputFileName);

      bytesToRead = Random.generate(1, InputBuffer.writable());

      if (false == InputBuffer.isWritable()) {
        if (1 < Debug)
          fprintf(stderr, "compacting input buffer for %s\n", inputFileName);

        if (false == InputBuffer.compact()) {
          fprintf(stderr, "cannot parse %s: parser jammed\n", inputFileName);
          return false;
        }
      }

      result = read(inputFd, InputBuffer.buffer() + InputBuffer.writePosition(),
                    bytesToRead);

      if (0 > result) {
        fprintf(stderr, "error reading %s: %s\n", inputFileName,
                strerror(errno));
        return false;
      }

      if (0 == result) {
        fprintf(stderr, "EOF reading %s\n", inputFileName);
        break;
      }

      InputBuffer.setWritePosition(InputBuffer.writePosition() + result);

      if (1 < Debug)
        fprintf(stderr, "read %ld bytes from file %s\n", (long int)result,
                inputFileName);

      continue;
    }

    if (ESB_SUCCESS != error) {
      fprintf(stderr, "error parsing %s: %d\n", inputFileName, error);
      return false;
    }

    if (0 == chunkSize) {
      if (Debug) fprintf(stderr, "\nfinished reading body %s\n", inputFileName);

      break;
    }

    if (Debug) {
      char buffer[sizeof(InputBufferStorage) + 1];

      memcpy(buffer, InputBuffer.buffer() + startingPosition, chunkSize);
      buffer[chunkSize] = 0;

      if (3 < Debug) fprintf(stderr, "read chunk:\n");
      if (Debug) fprintf(stderr, "%s", buffer);
    }

    bytesWritten = 0;

    while (bytesWritten < chunkSize) {
      error = ResponseFormatter.beginBlock(
          &OutputBuffer, chunkSize - bytesWritten, &availableSize);

      if (ESB_AGAIN == error) {
        if (1 < Debug)
          fprintf(stderr, "flushing output buffer for %s\n", outputFileName);

        if (false == OutputBuffer.isReadable()) {
          fprintf(stderr, "cannot format %s: formatter jammed\n",
                  outputFileName);
          return false;
        }

        while (OutputBuffer.isReadable()) {
          result =
              write(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

          if (0 > result) {
            fprintf(stderr, "error writing %s: %s\n", outputFileName,
                    strerror(errno));
            return false;
          }

          OutputBuffer.setReadPosition(result);
          OutputBuffer.compact();
        }

        continue;
      }

      if (ESB_SUCCESS != error) {
        fprintf(stderr, "error formatting %s: %d\n", outputFileName, error);
        return false;
      }

      memcpy(OutputBuffer.buffer() + OutputBuffer.writePosition(),
             InputBuffer.buffer() + startingPosition + bytesWritten,
             availableSize);

      OutputBuffer.setWritePosition(OutputBuffer.writePosition() +
                                    availableSize);

      bytesWritten += availableSize;

      while (true) {
        error = ResponseFormatter.endBlock(&OutputBuffer);

        if (ESB_AGAIN == error) {
          if (1 < Debug)
            fprintf(stderr, "flushing output buffer for %s\n", outputFileName);

          if (false == OutputBuffer.isReadable()) {
            fprintf(stderr, "cannot format %s: formatter jammed\n",
                    outputFileName);
            return false;
          }

          while (OutputBuffer.isReadable()) {
            result =
                write(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

            if (0 > result) {
              fprintf(stderr, "error writing %s: %s\n", outputFileName,
                      strerror(errno));
              return false;
            }

            OutputBuffer.setReadPosition(result);
            OutputBuffer.compact();
          }

          continue;
        }

        if (ESB_SUCCESS != error) {
          fprintf(stderr, "error formatting %s: %d\n", outputFileName, error);
          return false;
        }

        break;
      }
    }
  }

  // format last chunk

  while (true) {
    error = ResponseFormatter.endBody(&OutputBuffer);

    if (ESB_AGAIN == error) {
      if (1 < Debug)
        fprintf(stderr, "flushing output buffer for %s\n", outputFileName);

      if (false == OutputBuffer.isReadable()) {
        fprintf(stderr, "cannot format %s: formatter jammed\n", outputFileName);
        return false;
      }

      while (OutputBuffer.isReadable()) {
        result =
            write(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

        if (0 > result) {
          fprintf(stderr, "error writing %s: %s\n", outputFileName,
                  strerror(errno));
          return false;
        }

        OutputBuffer.setReadPosition(result);
        OutputBuffer.compact();
      }

      continue;
    }

    if (ESB_SUCCESS != error) {
      fprintf(stderr, "error formatting %s: %d\n", outputFileName, error);
      return false;
    }

    break;
  }

  // flush anything left in the output buffer

  if (1 < Debug)
    fprintf(stderr, "final flushing output buffer for %s\n", outputFileName);

  while (OutputBuffer.isReadable()) {
    result = write(outputFd, OutputBuffer.buffer(), OutputBuffer.readable());

    if (0 > result) {
      fprintf(stderr, "error writing %s: %s\n", outputFileName,
              strerror(errno));
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
      if (1 < Debug)
        fprintf(stderr, "need more data from file %s\n", inputFileName);

      bytesToRead = Random.generate(1, InputBuffer.writable());

      if (false == InputBuffer.isWritable()) {
        if (1 < Debug)
          fprintf(stderr, "compacting input buffer for %s\n", inputFileName);

        if (false == InputBuffer.compact()) {
          fprintf(stderr, "cannot parse %s: parser jammed\n", inputFileName);
          return false;
        }
      }

      result = read(inputFd, InputBuffer.buffer() + InputBuffer.writePosition(),
                    bytesToRead);

      if (0 > result) {
        fprintf(stderr, "error reading %s: %s\n", inputFileName,
                strerror(errno));
        return false;
      }

      if (0 == result) {
        fprintf(stderr, "premature EOF! %s\n", inputFileName);
        return false;
      }

      InputBuffer.setWritePosition(InputBuffer.writePosition() + result);

      if (1 < Debug)
        fprintf(stderr, "read %ld bytes from file %s\n", (long int)result,
                inputFileName);

      continue;
    }

    if (ESB_SUCCESS != error) {
      fprintf(stderr, "error parsing %s: %d\n", inputFileName, error);
      return false;
    }

    if (Debug) {
      fprintf(stderr, "trailer skipped for file %s\n", inputFileName);
    }

    break;
  }

  if (false == CompareFiles(outputFd, validationFd)) {
    fprintf(stderr, "%s does not match %s\n", outputFileName,
            validationFileName);
    return false;
  }

  fprintf(stderr, "Success!\n");

  return true;
}

bool CompareFiles(int fd1, int fd2) {
  if (0 > lseek(fd1, 0, SEEK_SET)) {
    fprintf(stderr, "cannot seek 1: %s\n", strerror(errno));
    return false;
  }

  if (0 > lseek(fd2, 0, SEEK_SET)) {
    fprintf(stderr, "cannot seek 2: %s\n", strerror(errno));
    return false;
  }

  char buffer1[4096];
  char buffer2[4096];
  ssize_t result1;
  ssize_t result2;

  while (true) {
    result1 = read(fd1, buffer1, sizeof(buffer1));

    if (0 > result1) {
      fprintf(stderr, "cannot read 1: %s\n", strerror(errno));
      return false;
    }

    if (0 == result1) {
      result1 = read(fd2, buffer2, sizeof(buffer2));

      if (0 > result1) {
        fprintf(stderr, "cannot read 2: %s\n", strerror(errno));
        return false;
      }

      if (0 == result1) {
        return true;
      }

      return false;
    }

    result2 = read(fd2, buffer2, result1);

    if (0 > result2) {
      fprintf(stderr, "cannot read 2: %s\n", strerror(errno));
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
  char currentWorkingDirectory[NAME_MAX + 1];

  if (0 == getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory))) {
    perror("Cannot get cwd");
    return 2;
  }

  DIR *directory = opendir(currentWorkingDirectory);

  for (struct dirent *entry = readdir(directory); entry;
       entry = readdir(directory)) {
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
