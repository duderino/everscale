#ifndef ESB_JSON_PARSER_H
#define ESB_JSON_PARSER_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ESB {

/**
 *  A SAX-style streaming JSON parser.
 *
 *  @defgroup json
 *  @ingroup json
 */
class JsonParser {
 public:
  JsonParser(Allocator &allocator = SystemAllocator::Instance());

  /** Destructor. */
  virtual ~JsonParser();

  /**
   * Parse a buffer full of JSON
   *
   * @param buffer The buffer to parse
   * @param size The size of the buffer to parse
   * @return ESB_SUCCESS if successful, ESB_BREAK if any on* functions returned BREAK, ESB_CANNOT_PARSE if invalid JSON
   * was encountered, another error code otherwise.
   */
  virtual Error parse(const unsigned char *buffer, UInt64 size);

  /**
   * Tell the parser to treat any buffered data as the end of the document.
   *
   * @return ESB_SUCCESS if successful, ESB_BREAK if any on* functions returned BREAK, ESB_CANNOT_PARSE if invalid JSON
   * was encountered, another error code otherwise.
   */
  virtual Error end();

  typedef enum { BREAK = 0, CONTINUE = 1 } ParseControl;

  virtual ParseControl onMapStart() = 0;

  virtual ParseControl onMapKey(const unsigned char *key, UInt32 length) = 0;

  virtual ParseControl onMapEnd() = 0;

  virtual ParseControl onArrayStart() = 0;

  virtual ParseControl onArrayEnd() = 0;

  virtual ParseControl onNull() = 0;

  virtual ParseControl onBoolean(bool value) = 0;

  virtual ParseControl onInteger(Int64 value) = 0;

  virtual ParseControl onDouble(double value) = 0;

  virtual ParseControl onString(const unsigned char *value, UInt32 length) = 0;

 private:
  void *_parser;

  typedef struct {
    void *ptr1;
    void *ptr2;
    void *ptr3;
    void *ptr4;
  } Opaque;
  Opaque _opaque;

  ESB_DISABLE_AUTO_COPY(JsonParser);
};

}  // namespace ESB

#endif
