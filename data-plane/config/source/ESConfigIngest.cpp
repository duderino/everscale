#ifndef ES_CONFIG_INGEST_H
#include <ESConfigIngest.h>
#endif

#ifndef ESB_BUFFERED_FILE_H
#include <ESBBufferedFile.h>
#endif

#ifndef ESB_JSON_PARSER_H
#include <ESBJsonParser.h>
#endif

#ifndef ESB_AST_TREE_H
#include <ASTTree.h>
#endif

namespace ES {

ConfigIngest::ConfigIngest(ESB::Allocator &allocator) : _allocator(allocator) {}

ConfigIngest::~ConfigIngest() {}

ESB::Error ConfigIngest::parse(const char *path) {
  if (!path) {
    return ESB_NULL_POINTER;
  }

  ESB::BufferedFile file(path, ESB::BufferedFile::READ_ONLY);
  unsigned char buffer[1024];
  ESB::AST::Tree tree(_allocator);
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

}  // namespace ES
