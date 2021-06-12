#ifndef ESB_BUFFERED_FILE_H
#define ESB_BUFFERED_FILE_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

namespace ESB {

/** BufferedFile is a blocking and buffered file reader.
 *
 *  @defgroup file
 *  @ingroup file
 */
class BufferedFile {
 public:
  typedef enum { READ_ONLY = 0 } Mode;

  /** Construct a buffered file
   *
   * @param path The path to the file to open
   * @param mode The file will be opened with this mode
   */
  BufferedFile(const char *path, Mode mode);

  /** Close the file if it hasn't been closed already
   */
  virtual ~BufferedFile();

  /**
   * Read from the file, opening it as a side effect if not already open.
   *
   * @param buffer The buffer to read into
   * @param size The size of the buffer
   * @param bytesRead The number of bytes actually read
   * @return ESB_SUCCESS if the requested size has been fully read, ESB_BREAK if the end of the file has been reached
   * (NB: check bytesRead here since partial reads are probable), another error code otherwise.
   */
  Error read(unsigned char *buffer, Size size, Size *bytesRead);

  /**
   * Read from the file, opening it as a side effect if not already open.
   *
   * @param buffer The buffer to read into.  This will attempt to fill the entire remaining space in the buffer.
   * @param bytesRead The number of bytes actually read
   * @return ESB_SUCCESS if the buffer has been completely filled, ESB_BREAK if the end of the file has been reached
   * (NB: check bytesRead here since partial reads are probable), another error code otherwise.
   */
  Error read(Buffer &buffer, Size *bytesRead);

  /** Close the file.
   */
  void close();

 private:
#ifdef HAVE_FILE_T
  FILE *_file;
#else
#error "FILE * or equivalent is required"
#endif
  const char *_path;
  Mode _mode;

  ESB_DEFAULT_FUNCS(BufferedFile);
};

}  // namespace ESB

#endif
