#ifndef ES_CONFIG_TEST_H
#include "ESConfigTest.h"
#endif

#ifndef ESB_BUFFERED_FILE_H
#include <ESBBufferedFile.h>
#endif

#ifndef ESB_JSON_PARSER_H
#include <ESBJsonParser.h>
#endif

namespace ES {

ConfigTest::ConfigTest(ESB::Allocator &allocator) : _allocator(allocator) {}

ConfigTest::~ConfigTest() {}

ESB::Error ConfigTest::parseFile(const char *path, ESB::AST::Tree &tree) {
  if (!path) {
    return ESB_NULL_POINTER;
  }

  ESB::BufferedFile file(path, ESB::BufferedFile::READ_ONLY);
  unsigned char buffer[1024];
  ESB::JsonParser parser(tree, _allocator);

  while (true) {
    ESB::Size bytesRead = 0;
    switch (ESB::Error error = file.read(buffer, sizeof(buffer), &bytesRead)) {
      case ESB_SUCCESS:
        error = parser.parse(buffer, bytesRead);
        if (ESB_SUCCESS != error) {
          return error;
        }
        break;
      case ESB_BREAK:
        if (0 < bytesRead) {
          error = parser.parse(buffer, bytesRead);
          if (ESB_SUCCESS != error) {
            return error;
          }
        }
        return parser.end();
      default:
        return error;
    }
  }
}

ESB::Error ConfigTest::parseString(const char *str, ESB::AST::Tree &tree) {
  ESB::JsonParser parser(tree, _allocator);
  ESB::Error error = parser.parse((const unsigned char *)str, strlen(str));
  return ESB_SUCCESS == error ? parser.end() : error;
}

}  // namespace ES
