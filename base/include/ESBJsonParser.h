#ifndef ESB_JSON_PARSER_H
#define ESB_JSON_PARSER_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_JSON_CALLBACKS_H
#include <ESBJsonCallbacks.h>
#endif

namespace ESB {

/**
 *  A SAX-style streaming JSON parser.
 *
 *  @ingroup json
 */
class JsonParser {
 public:
  JsonParser(JsonCallbacks &callbacks, Allocator &allocator = SystemAllocator::Instance());

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
   * Finishing parsing the document, finalizing the parse of any buffered data.
   *
   * @return ESB_SUCCESS if successful, ESB_BREAK if any on* functions returned BREAK, ESB_CANNOT_PARSE if invalid JSON
   * was encountered, another error code otherwise.
   */
  virtual Error end();

 private:
  typedef struct {
    void *ptr1;
    void *ptr2;
    void *ptr3;
    void *ptr4;
  } Opaque;

  void *_parser;
  Opaque _opaque;
  JsonCallbacks &_callbacks;

  ESB_DEFAULT_FUNCS(JsonParser);
};

}  // namespace ESB

#endif
