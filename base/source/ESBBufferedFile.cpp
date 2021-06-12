#ifndef ESB_BUFFERED_FILE_H
#include <ESBBufferedFile.h>
#endif

namespace ESB {

BufferedFile::BufferedFile(const char *path, Mode mode) : _file(NULL), _path(path), _mode(mode) {}

BufferedFile::~BufferedFile() { close(); }

Error BufferedFile::read(unsigned char *buffer, Size size, Size *bytesRead) {
  if (!buffer || !bytesRead) {
    return ESB_NULL_POINTER;
  }

  if (!_file) {
    assert(READ_ONLY == _mode);
#ifdef HAVE_FOPEN
    _file = fopen(_path, "r");
    if (!_file) {
      return LastError();
    }
#else
#error "fopen or equivalent is required"
#endif
  }

  Size result = fread(buffer, 1, size, _file);
  if (result != size) {
    if (feof(_file)) {
      *bytesRead = result;
      return ESB_BREAK;
    }
    if (ferror(_file)) {
      return LastError();
    }
  }

  *bytesRead = result;
  return ESB_SUCCESS;
}

Error BufferedFile::read(Buffer &buffer, Size *bytesRead) {
  switch (Error error = read(buffer.buffer() + buffer.writePosition(), buffer.writable(), bytesRead)) {
    case ESB_SUCCESS:
    case ESB_BREAK:
      buffer.setWritePosition(buffer.writePosition() + *bytesRead);
    default:
      return error;
  }
}

void BufferedFile::close() {
  if (_file) {
#ifdef HAVE_FCLOSE
    fclose(_file);
#else
#error "fclose or equivalent is required"
#endif
    _file = NULL;
  }
}

}  // namespace ESB
